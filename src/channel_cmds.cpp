#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string_view	target(client.parser.params[0]);
	std::string_view	message(client.parser.params[1]);

	if (target.empty())
	{
		build_ERR_NOSUCHNICK(client, target);
		return;
	}

	std::string		line = ":";
	append_nick_user_host(line, client);
	line += " PRIVMSG ";

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

		line += channel->name;
		line += " :";
		line += message;
		line += "\r\n";

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

		target_client->send_message_buffer += line;
		target_client->send_message_buffer += target_client->nick;
		target_client->send_message_buffer += " :";
		target_client->send_message_buffer += message;
		target_client->send_message_buffer += "\r\n";
	}
}

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		channel_name(client.parser.params[0]);

	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&'))
	{
		build_ERR_BADCHANMASK(client, channel_name);
		return;
	}

	bool			just_created = false;
	auto			ch_it = server.channels.find(channel_name);
	if (ch_it == server.channels.end())
	{
		if (server.channels.size() >= MAX_CHANNELS)
		{
			build_ERR_TOOMANYCHANNELS(client, channel_name);
			return;
		}
		ch_it = server.channels.emplace(channel_name, t_IRC_Channel{}).first;
		ch_it->second.name = channel_name;
		ch_it->second.mode |= TOPIC;
		just_created = true;
	}

	t_IRC_Channel	&channel = ch_it->second;

	if (channel.members.contains(&client))
		return;

	if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
	{
		build_ERR_TOOMANYCHANNELS(client, channel.name);
		return;
	}

	if (is_flag_set(channel.mode, LIMIT) && channel.members.size() >= channel.user_limit)
   	{
		build_ERR_CHANNELISFULL(client, channel.name);
		return;
	}

	if (is_flag_set(channel.mode, INVITE) && !channel.invited.contains(&client))
	{
		build_ERR_INVITEONLYCHAN(client, channel.name);
		return;
	}

	if (is_flag_set(channel.mode, KEY))
	{
		if (client.parser.n_params < 2 || channel.key != client.parser.params[1])
		{
			build_ERR_BADCHANNELKEY(client, channel.name);
			return;
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

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
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

	if (!channel->members.contains(&client))
	{
		build_ERR_NOTONCHANNEL(client, channel->name);
		return;
	}

	std::string		line;
	append_PART_msg(line, client, channel->name);
	broadcast_to_channel(*channel, line, client, false);
	remove_client_from_channel(client, *channel, server);
}

void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(kicker);
		return;
	}
	std::string_view	channel_name(kicker.parser.params[0]);
	std::string_view	victim(kicker.parser.params[1]);
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

	t_IRC_Client	*to_be_kicked = find_chmember_by_nick(*channel, victim);
	if (!to_be_kicked)
	{
		build_ERR_USERNOTINCHANNEL(kicker, channel->name, victim);
		return;
	}

	std::string line;
	std::string_view reason;
	if (kicker.parser.n_params >= 3) //there's a reason specified
		reason = kicker.parser.params[2];
	append_KICK_msg(line, kicker, channel->name, to_be_kicked->nick, reason);
	broadcast_to_channel(*channel, line, kicker, false);
	remove_client_from_channel(*to_be_kicked, *channel, server);
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
	if (client.parser.n_params < 1)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string_view	channel_name(client.parser.params[0]);
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_RPL_ENDOFNAMES(client, channel_name);
		return;
	}

	send_names_reply(client, *channel);
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
		size_t				start = 0;

		while (start <= targets.size())
		{
			size_t			end = targets.find(',', start);
			if (end == std::string_view::npos)
				end = targets.size();

			std::string		name(targets.substr(start, end - start));
			t_IRC_Channel	*channel = find_channel_by_name(server, name);
			if (channel)
				build_RPL_LIST(client, *channel);
			start = end + 1;
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
