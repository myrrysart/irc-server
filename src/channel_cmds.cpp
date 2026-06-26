#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <charconv>
#include <string>

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		target(client.parser.params[0]);
	std::string		message(client.parser.params[1]);

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

		line += target_client->nick;
		line += " :";
		line += message;
		line += "\r\n";

		target_client->send_message_buffer += line;
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

	//validate channel name
	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&'))
	{
		build_ERR_BADCHANMASK(client, channel_name);
		return;
	}

	// Find existing channel, or create a new one
	auto			ch_it = server.channels.find(channel_name);
	if (ch_it == server.channels.end())
	{
		if (server.channels.size() >= MAX_CHANNELS)
		{
			build_ERR_TOOMANYCHANNELS(client, channel_name);
			return;
		}
		ch_it = server.channels.emplace(channel_name, t_IRC_Channel{}).first;
	}

	t_IRC_Channel	&channel = ch_it->second;

	// Set channel.name on first creation
	if (channel.name.empty())
		channel.name = channel_name;

	// Already a member -> return
	if (channel.members.contains(&client))
		return;

	// limit check
	if (is_flag_set(channel.mode, LIMIT) && channel.members.size() >= static_cast<size_t>(channel.user_limit))
   	{
		build_ERR_CHANNELISFULL(client, channel_name);
		return;
	}
	// invite check
	if (is_flag_set(channel.mode, INVITE) && !channel.invited.contains(&client))
	{
		build_ERR_INVITEONLYCHAN(client, channel_name);
		return;
	}
	// key check
	if (is_flag_set(channel.mode, KEY))
	{
		if (client.parser.n_params < 2 || channel.key != client.parser.params[1])
		{
			build_ERR_BADCHANNELKEY(client, channel_name);
			return;
		}
	}
	// Build member flags. first joiner becomes channel operator
	t_bmask			flags = 0;
	if (channel.members.empty())
	{
		flags |= IS_OPERATOR;
		channel.mode |= TOPIC;
	}
	// Record membership on channel and client
	channel.members[&client] = flags;
	client.joined_channels.insert(&channel);

	std::string		line;
	append_JOIN_msg(line, client, channel_name);
	broadcast_to_channel(channel, line, client, false);
}

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		channel_name(client.parser.params[0]);

	t_IRC_Channel	*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	if (!channel->members.contains(&client))
	{
		build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}

	std::string		line;
	append_PART_msg(line, client, channel_name);
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
	std::string		channel_name(kicker.parser.params[0]);
	std::string		victim(kicker.parser.params[1]);
	t_IRC_Channel	*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(kicker, channel_name);
		return;
	}

	auto			kicker_it = channel->members.find(&kicker);
	if (kicker_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(kicker, channel_name);
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(kicker, channel_name);
		return;
	}

	t_IRC_Client	*to_be_kicked = find_chmember_by_nick(*channel, victim);
	if (!to_be_kicked)
	{
		build_ERR_USERNOTINCHANNEL(kicker, channel_name, victim);
		return;
	}

	std::string line;
	std::string_view reason;
	if (kicker.parser.n_params >= 3) //there's a reason specified
		reason = kicker.parser.params[2];
	append_KICK_msg(line, kicker, channel_name, to_be_kicked->nick, reason);
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

	std::string			channel_name(client.parser.params[0]);
	std::string_view	topic = client.parser.params[1];

	server.channels[channel_name].topic = topic;

	std::string			line;
	append_TOPIC_msg(line, client, topic);
	broadcast_to_channel(server.channels[channel_name], line, client, false);

}

void execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 1)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string channel_name(client.parser.params[0]);
	t_IRC_Channel *channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	std::string line;
	for (const auto &[member, flags] : channel->members)
	{
		line += member->nick;
		line += ' ';
	}
	build_RPL_NAMES(client, line);
	build_RPL_ENDOFNAMES(client, channel_name);
}

void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	std::string	line;
	for (const auto &[name, channel] : server.channels)
		build_RPL_LIST(client, channel);
	build_RPL_LISTEND(client);
}

void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string		target_nick(client.parser.params[0]);
	std::string		channel_name(client.parser.params[1]);
	t_IRC_Channel	*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	t_IRC_Client *target = find_client_by_nick(server, target_nick);
	if (!target)
	{
		build_ERR_NOSUCHNICK(client, target_nick);
		return;
	}

	channel->invited.insert(target);
	build_RPL_INVITING(client, target_nick, channel_name);
}
