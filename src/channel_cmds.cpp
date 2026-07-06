#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NORECIPIENT(client);
		return;
	}
	// WARN: n_params==1 cannot distinguish PRIVMSG bob (412) from PRIVMSG :hello (411);
	// Same param, so needs parser input to return 411 for trailing-only lines.
	// Is this needed in practice? Irssi invokes just 412
	if (client.parser.n_params < 2)
	{
		build_ERR_NOTEXTTOSEND(client); //412
		return;
	}

	std::string_view	targets(client.parser.params[0]);
	std::string_view	message(client.parser.params[1]);
	size_t				pos = 0;
	bool				delivered = false;

	while (pos <= targets.size())
	{
		std::string_view	target = next_comma_token(targets, pos);
		if (target.empty())
			continue;

		if (target[0] == '#' || target[0] == '&')
		{
			t_IRC_Channel	*channel = find_channel_by_name(server, target);
			if (!channel)
			{
				build_ERR_NOSUCHCHANNEL(client, target);
				return;
			}
			if (!channel->members.contains(&client))
			{
				build_ERR_CANNOTSENDTOCHAN(client, target);
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, channel->name, message);
			broadcast_to_channel(*channel, line, client, true);
		}
		else
		{
			t_IRC_Client	*target_client = find_client_by_nick(server, target);
			if (!target_client)
			{
				build_ERR_NOSUCHNICK(client, target);
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, target_client->nick, message);
			target_client->send_message_buffer += line;
		}
		delivered = true;
	}
	if (!delivered)
		build_ERR_NORECIPIENT(client);
}

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string_view	channels(client.parser.params[0]);
	std::string_view	keys;
	size_t				channel_pos = 0;
	size_t				key_pos = 0;
	bool				has_channel = false;

	if (client.parser.n_params >= 2)
		keys = client.parser.params[1];
	while (channel_pos <= channels.size())
	{
		std::string_view	channel_token = next_comma_token(channels, channel_pos);
		std::string_view	key;

		if (client.parser.n_params >= 2 && key_pos <= keys.size())
			key = next_comma_token(keys, key_pos);
		if (channel_token.empty())
			continue;
		has_channel = true;

		std::string	channel_name(channel_token);
		if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&'))
		{
			build_ERR_BADCHANMASK(client, channel_name);
			continue;
		}

		bool			just_created = false;
		auto			ch_it = server.channels.find(channel_name);
		if (ch_it != server.channels.end() && ch_it->second.members.contains(&client))
			continue;

		if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
		{
			build_ERR_TOOMANYCHANNELS(client, channel_name);
			continue;
		}

		if (ch_it == server.channels.end())
		{
			if (server.channels.size() >= MAX_CHANNELS)
			{
				build_ERR_TOOMANYCHANNELS(client, channel_name);
				continue;
			}
			ch_it = server.channels.emplace(channel_name, t_IRC_Channel{}).first;
			ch_it->second.name = channel_name;
			ch_it->second.mode |= TOPIC;
			just_created = true;
		}

		t_IRC_Channel	&channel = ch_it->second;

		if (is_flag_set(channel.mode, LIMIT) && channel.members.size() >= channel.user_limit)
		{
			build_ERR_CHANNELISFULL(client, channel.name);
			continue;
		}

		if (is_flag_set(channel.mode, INVITE) && !channel.invited.contains(&client))
		{
			build_ERR_INVITEONLYCHAN(client, channel.name);
			continue;
		}

		if (is_flag_set(channel.mode, KEY))
		{
			if (key.empty() || channel.key != key)
			{
				build_ERR_BADCHANNELKEY(client, channel.name);
				continue;
			}
		}

		t_bmask			join_flags = 0;
		if (just_created)
			join_flags |= IS_OPERATOR;

		channel.members[&client] = join_flags;
		client.joined_channels.insert(&channel);
		channel.invited.erase(&client);

		std::string		line;
		append_JOIN_msg(line, client, channel.name);
		broadcast_to_channel(channel, line, client, false);
		if (!channel.topic.empty())
			build_RPL_TOPIC(client, channel);
		send_names_reply(client, channel);
	}
	if (!has_channel)
		build_ERR_BADCHANMASK(client, channels);
}

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string_view	channels(client.parser.params[0]);
	std::string_view	reason;
	size_t				channel_pos = 0;
	bool				has_channel = false;

	if (client.parser.n_params >= 2)
		reason = client.parser.params[1];
	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		has_channel = true;

		t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
		if (!channel)
		{
			build_ERR_NOSUCHCHANNEL(client, channel_name);
			continue;
		}

		if (!channel->members.contains(&client))
		{
			build_ERR_NOTONCHANNEL(client, channel->name);
			continue;
		}

		std::string		line;
		append_PART_msg(line, client, channel->name, reason);
		broadcast_to_channel(*channel, line, client, false);
		remove_client_from_channel(client, *channel, server);
	}
	if (!has_channel)
		build_ERR_NEEDMOREPARAMS(client);
}

void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(kicker);
		return;
	}
	std::string_view	channel_name(kicker.parser.params[0]);
	std::string_view	victims(kicker.parser.params[1]);
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(kicker, channel_name);
		return;
	}

	auto			kicker_it = channel->members.find(&kicker);
	if (kicker_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(kicker, channel->name);
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(kicker, channel->name);
		return;
	}

	std::string_view reason;
	if (kicker.parser.n_params >= 3) //there's a reason specified
		reason = kicker.parser.params[2];
	size_t				victim_pos = 0;
	bool				has_victim = false;

	while (victim_pos <= victims.size())
	{
		std::string_view	victim = next_comma_token(victims, victim_pos);
		if (victim.empty())
			continue;
		has_victim = true;

		t_IRC_Client	*to_be_kicked = find_chmember_by_nick(*channel, victim);
		if (!to_be_kicked)
		{
			build_ERR_USERNOTINCHANNEL(kicker, channel->name, victim);
			continue;
		}

		std::string	line;
		append_KICK_msg(line, kicker, channel->name, to_be_kicked->nick, reason);
		broadcast_to_channel(*channel, line, kicker, false);
		remove_client_from_channel(*to_be_kicked, *channel, server);
		if (to_be_kicked == &kicker)
			return;
	}
	if (!has_victim)
		build_ERR_NEEDMOREPARAMS(kicker);
}

void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 1)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string_view	channel_name(client.parser.params[0]);
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	auto	member_it = channel->members.find(&client);
	if (member_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel->name);
		return;
	}

	if (client.parser.n_params < 2)
	{
		if (channel->topic.empty())
			build_RPL_NOTOPIC(client, channel->name);
		else
			build_RPL_TOPIC(client, *channel);
		return;
	}

	if (is_flag_set(channel->mode, TOPIC) && !is_flag_set(member_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel->name);
		return;
	}
	channel->topic.assign(client.parser.params[1]); // copy out of the receive buffer

	std::string	line;
	append_TOPIC_msg(line, client, channel->name, channel->topic);
	broadcast_to_channel(*channel, line, client, false);
}


void execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		auto	channel_it = server.channels.begin();
		while (channel_it != server.channels.end())
		{
			send_names_reply(client, channel_it->second);
			++channel_it;
		}
		if (server.channels.empty())
			build_RPL_ENDOFNAMES(client, "*");
		return;
	}

	std::string_view	channels(client.parser.params[0]);
	size_t				channel_pos = 0;
	bool				has_channel = false;

	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		has_channel = true;

		t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
		if (!channel)
		{
			build_RPL_ENDOFNAMES(client, channel_name);
			continue;
		}
		send_names_reply(client, *channel);
	}
	if (!has_channel)
		build_RPL_ENDOFNAMES(client, "*");
}

void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	//No target
	if (client.parser.n_params == 0)
	{
		for (const auto &[name, channel] : server.channels)
			build_RPL_LIST(client, channel);
	}
	else
	{
		std::string_view	targets = client.parser.params[0];
		size_t				pos = 0;

		while (pos <= targets.size())
		{
			std::string_view	name = next_comma_token(targets, pos);
			if (name.empty())
				continue;
			t_IRC_Channel	*channel = find_channel_by_name(server, name);
			if (channel)
				build_RPL_LIST(client, *channel);
		}
	}
	build_RPL_LISTEND(client);
}


void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string_view	target_nick(client.parser.params[0]);
	std::string_view	channel_name(client.parser.params[1]);
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	auto	inviter_it = channel->members.find(&client);
	if (inviter_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel->name);
		return;
	}
	if (!is_flag_set(inviter_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel->name);
		return;
	}

	t_IRC_Client *target = find_client_by_nick(server, target_nick);
	if (!target)
	{
		build_ERR_NOSUCHNICK(client, target_nick);
		return;
	}

	if (channel->members.contains(target))
	{
		build_ERR_USERONCHANNEL(client, target_nick, channel->name);
		return;
	}

	channel->invited.insert(target);
	build_RPL_INVITING(client, target_nick, channel->name);

	std::string		line;
	append_INVITE_msg(line, client, target_nick, channel->name);
	target->send_message_buffer += line;
}
