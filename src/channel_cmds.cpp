#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no target at all
	if (client.parser.n_params == 0)
	{
		build_ERR_NORECIPIENT(client); // 411
		return;
	}
	// WARN: n_params==1 cannot distinguish PRIVMSG bob (412) from PRIVMSG :hello (411);
	// Same param, so needs parser input to return 411 for trailing-only lines.
	// Is this needed in practice? Irssi invokes just 412
	// target given but no message text
	if (client.parser.n_params < 2)
	{
		build_ERR_NOTEXTTOSEND(client); // 412
		return;
	}

	std::string_view	targets(client.parser.params[0]);
	std::string_view	message(client.parser.params[1]);
	size_t				pos = 0;
	bool				any_message_delivered = false;

	while (pos <= targets.size())
	{
		std::string_view	target = next_comma_token(targets, pos);
		if (target.empty())
			continue;

		// channel target: must exist and client must be a member
		if (target[0] == '#' || target[0] == '&')
		{
			std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
				find_channel_by_name(server, target);
			if (ch_it == server.channels.end())
			{
				build_ERR_NOSUCHCHANNEL(client, target); // 403
				return;
			}
			t_IRC_Channel	&channel = ch_it->second;
			if (!channel.members.contains(&client))
			{
				build_ERR_CANNOTSENDTOCHAN(client, target); // 404
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, channel.name, message);
			broadcast_to_channel(channel, line, client, true);
		}
		// nick target: must be an online client
		else
		{
			trim_nickname_if_longer_than_max_nicklen(target);
			t_IRC_Client	*target_client = find_client_by_nick(server, target);
			if (!target_client)
			{
				build_ERR_NOSUCHNICK(client, target); // 401
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, target_client->nick, message);
			target_client->send_message_buffer += line;
		}
		any_message_delivered = true;
	}
	// catches "targets was just commas" (e.g. PRIVMSG , :hi)
	if (!any_message_delivered)
		build_ERR_NORECIPIENT(client); // 411
}

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// "0" is a special JOIN argument
	if (client.parser.params[0] == "0")
	{
		remove_client_from_all_channels(client, server);
		return;
	}

	std::string_view	channels(client.parser.params[0]);
	std::string_view	keys;
	size_t				channel_pos = 0;
	size_t				key_pos = 0;

	// if there are more params than one, we consider them keys for the channels
	if (client.parser.n_params >= 2)
		keys = client.parser.params[1];
	// each parsed request pairs one channel token with the next key token (if any)
	while (channel_pos <= channels.size())
	{
		// directly initialize the struct's members, without intermediate steps
		t_key_channel	req{ next_comma_token(channels, channel_pos), {} };
		// key/channel pairing — RFC pairs keys by index for every
		// channel in JOIN ch1,ch2 k1,k2
		// Even if the channel is an invalid one, the pairing still holds.
		if (client.parser.n_params >= 2 && key_pos <= keys.size())
			req.key = next_comma_token(keys, key_pos);

		if (req.channel.size() < 2 || (req.channel[0] != '#' && req.channel[0] != '&'))
		{
			build_ERR_BADCHANMASK(client, req.channel); // 476
			continue;
		}

		std::string	channel_name(req.channel);
		auto		ch_it = server.channels.find(channel_name);
		t_bmask		join_flags = 0;
		bool		channel_just_created = false;
		bool		channel_has_ops = false;

		if (ch_it == server.channels.end()) // new channel to be created
		{
			// doesn't exist yet. Create it, first joiner becomes operator
			// but first: check for channel limits
			if (server.channels.size() >= MAX_CHANNELS)
			{
				build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
				continue;
			}
			if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
			{
				build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
				continue;
			}

			ch_it = server.channels.emplace(channel_name, t_IRC_Channel{}).first;
			ch_it->second.name = channel_name;
			ch_it->second.mode |= TOPIC;
			channel_just_created = true;
		}
		else // existing channel
		{
			// already in it. Silent.
			if (ch_it->second.members.contains(&client))
				continue;

			// check all flags

			if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
			{
				build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
				continue;
			}

			if (is_flag_set(ch_it->second.mode, LIMIT)
				&& ch_it->second.members.size() >= ch_it->second.user_limit)
			{
				build_ERR_CHANNELISFULL(client, ch_it->second.name); // 471
				continue;
			}

			if (is_flag_set(ch_it->second.mode, INVITE)
				&& !ch_it->second.invited.contains(&client))
			{
				build_ERR_INVITEONLYCHAN(client, ch_it->second.name); // 473
				continue;
			}

			if (is_flag_set(ch_it->second.mode, KEY))
			{
				if (req.key.empty() || ch_it->second.key != req.key)
				{
					build_ERR_BADCHANNELKEY(client, ch_it->second.name); // 475
					continue;
				}
			}
		}
		t_IRC_Channel	&channel = ch_it->second;

		// if channel has no ops, you are the op
		for (const auto &[member, member_flags] : channel.members)
		{
			if (is_flag_set(member_flags, IS_OPERATOR))
			{
				channel_has_ops = true;
				break;
			}
		}

		// all checks passed. record membership and broadcast
		if (!channel_has_ops)
			join_flags |= IS_OPERATOR;
		if (channel_just_created)
			join_flags |= IS_OPERATOR;

		channel.members[&client] = join_flags;
		client.joined_channels.insert(&channel);
		channel.invited.erase(&client); // consume any pending invite

		std::string		line;
		append_JOIN_msg(line, client, channel.name);
		broadcast_to_channel(channel, line, client, false);
		if (!channel.topic.empty())
			build_RPL_TOPIC(client, channel); // 332
		send_names_reply(client, channel);
	}
}

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}
	std::string_view	channels(client.parser.params[0]);
	std::string_view	reason;
	size_t				channel_pos = 0;
	bool				has_nonempty_channel_token = false;

	if (client.parser.n_params >= 2)
		reason = client.parser.params[1];
	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		has_nonempty_channel_token = true;

		// channel must exist and client must be in it
		std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
			find_channel_by_name(server, channel_name);
		if (ch_it == server.channels.end())
		{
			build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
			continue;
		}

		t_IRC_Channel	&channel = ch_it->second;

		if (!channel.members.contains(&client))
		{
			build_ERR_NOTONCHANNEL(client, channel.name); // 442
			continue;
		}

		// broadcast to  members, then erase membership
		std::string		line;
		append_PART_msg(line, client, channel.name, reason);
		broadcast_to_channel(channel, line, client, false);
		remove_client_from_channel(client, channel, server);
	}
	if (!has_nonempty_channel_token)
		build_ERR_NEEDMOREPARAMS(client); // 461
}

void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(kicker); // 461
		return;
	}
	std::string_view	channel_name(kicker.parser.params[0]);
	std::string_view	victims(kicker.parser.params[1]);
	std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
		find_channel_by_name(server, channel_name);
	if (ch_it == server.channels.end())
	{
		build_ERR_NOSUCHCHANNEL(kicker, channel_name); // 403
		return;
	}

	t_IRC_Channel	&channel = ch_it->second;

	// kicker must be in the channel and an operator
	auto			kicker_it = channel.members.find(&kicker);
	if (kicker_it == channel.members.end())
	{
		build_ERR_NOTONCHANNEL(kicker, channel.name); // 442
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(kicker, channel.name); // 482
		return;
	}

	std::string_view reason;
	if (kicker.parser.n_params >= 3) //there's a reason specified
		reason = kicker.parser.params[2];
	size_t				victim_pos = 0;
	bool				has_victim_token = false;

	while (victim_pos <= victims.size())
	{
		std::string_view	victim = next_comma_token(victims, victim_pos);
		if (victim.empty())
			continue;
		has_victim_token = true;
		trim_nickname_if_longer_than_max_nicklen(victim);

		// each victim must currently be in the channel
		t_IRC_Client	*to_be_kicked = find_chmember_by_nick(channel, victim);
		if (!to_be_kicked)
		{
			build_ERR_USERNOTINCHANNEL(kicker, channel.name, victim); // 441
			continue;
		}

		// notify, then remove.
		std::string	line;
		append_KICK_msg(line, kicker, channel.name, to_be_kicked->nick, reason);
		broadcast_to_channel(channel, line, kicker, false);
		remove_client_from_channel(*to_be_kicked, channel, server);
		// self-kick.  Channel/members may be gone, stop here
		if (to_be_kicked == &kicker)
			return;
	}
	if (!has_victim_token)
		build_ERR_NEEDMOREPARAMS(kicker); // 461
}

void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 1)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// client must be a member to see/set its topic
	std::string_view	channel_name(client.parser.params[0]);

	std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
		find_channel_by_name(server, channel_name);

	if (ch_it == server.channels.end())
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
		return;
	}

	t_IRC_Channel	&channel = ch_it->second;

	auto	member_it = channel.members.find(&client);
	if (member_it == channel.members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel.name); // 442
		return;
	}

	// no second param
	if (client.parser.n_params < 2)
	{
		if (channel.topic.empty())
			build_RPL_NOTOPIC(client, channel.name); // 331
		else
			build_RPL_TOPIC(client, channel); // 332
		return;
	}

	// +t topic change for operators only
	if (is_flag_set(channel.mode, TOPIC) && !is_flag_set(member_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel.name); // 482
		return;
	}
	channel.topic.assign(client.parser.params[1]); // copy out of the receive buffer

	std::string	line;
	append_TOPIC_msg(line, client, channel.name, channel.topic);
	broadcast_to_channel(channel, line, client, false);
}


void execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no params -> broadcast every channel on the server
	if (client.parser.n_params == 0)
	{
		auto	channel_it = server.channels.begin();
		while (channel_it != server.channels.end())
		{
			send_names_reply(client, channel_it->second);
			++channel_it;
		}
		if (server.channels.empty())
			build_RPL_ENDOFNAMES(client, "*"); // 366
		return;
	}

	// params  only the requested channels -> unknown ones get ENDOFNAMES
	std::string_view	channels(client.parser.params[0]);
	size_t				channel_pos = 0;
	bool				saw_nonempty_channel_toke = false;

	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		saw_nonempty_channel_toke = true;


		std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
			find_channel_by_name(server, channel_name);
		if (ch_it == server.channels.end())
		{
			build_RPL_ENDOFNAMES(client, channel_name); // 366
			continue;
		}
		send_names_reply(client, ch_it->second);
	}
	if (!saw_nonempty_channel_toke)
		build_RPL_ENDOFNAMES(client, "*"); // 366
}

void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	//No target
	if (client.parser.n_params == 0)
	{
		// no filter -> list every channel
		for (const auto &[name, channel] : server.channels)
			build_RPL_LIST(client, channel); // 322
	}
	else
	{
		// filter -> only list the requested channels
		std::string_view	targets = client.parser.params[0];
		size_t				pos = 0;

		while (pos <= targets.size())
		{
			std::string_view	name = next_comma_token(targets, pos);
			if (name.empty())
				continue; //skip empty tokens

			std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
				find_channel_by_name(server, name);
			if (ch_it != server.channels.end())
				build_RPL_LIST(client, ch_it->second); // 322
		}
	}
	build_RPL_LISTEND(client); // 323
}

void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// find channel, inviter must be a member and an operator
	std::string_view	target_nick(client.parser.params[0]);
	trim_nickname_if_longer_than_max_nicklen(target_nick);
	std::string_view	channel_name(client.parser.params[1]);

	std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it =
		find_channel_by_name(server, channel_name);
	if (ch_it == server.channels.end())
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
		return;
	}

	t_IRC_Channel	&channel = ch_it->second;

	auto	inviter_it = channel.members.find(&client);
	if (inviter_it == channel.members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel.name); // 442
		return;
	}
	if (!is_flag_set(inviter_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel.name); // 482
		return;
	}

	// target must exist and not already be a member
	t_IRC_Client *target = find_client_by_nick(server, target_nick);
	if (!target)
	{
		build_ERR_NOSUCHNICK(client, target_nick); // 401
		return;
	}

	if (channel.members.contains(target))
	{
		build_ERR_USERONCHANNEL(client, target->nick, channel.name); // 443
		return;
	}

	// record invite (bypasses +i) and notify both sides
	channel.invited.insert(target);
	build_RPL_INVITING(client, target->nick, channel.name); // 341

	std::string		line;
	append_INVITE_msg(line, client, target->nick, channel.name);
	target->send_message_buffer += line;
}
