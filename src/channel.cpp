#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, const std::string_view nick)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (are_equal_strs_case_insensitive(nick.data(), nick.size(), member_ptr->nick.data(), member_ptr->nick.size()))
			return member_ptr;
	}
	return nullptr;
}

t_IRC_Channel	*find_channel_by_name(t_IRC_Server &server, std::string ch_name)
{
	if (auto it = server.channels.find(ch_name); it != server.channels.end())
		return &it->second;
	return nullptr;
}

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		target(client.parser.params[0]);
	std::string		message(client.parser.params[1]);

	std::string		line;
	line += ':';
	line += client.nick;
	line += '!';
	line += client.username;
	line += '@';
	line += client.hostname;
	line += " PRIVMSG ";

	if (target[0] == '#' || target[0] == '&')
	{
		auto	ch_it = server.channels.find(target);
		if (ch_it == server.channels.end())
		{
			// TODO: build_ERR_NOSUCHCHANNEL(client, target);
			return;
		}
		t_IRC_Channel	&channel = ch_it->second;

		if (!channel.members.contains(&client))
		{
			// TODO: build_ERR_CANNOTSENDTOCHAN(client, target);
			return;
		}

		line += channel.name;
		line += " :";
		line += message;
		line += "\r\n";

		for (auto &[member_ptr, flags] : channel.members)
			member_ptr->send_message_buffer += line;
	}
	else
	{
		t_IRC_Client	*target_client = nullptr;

		for (auto &[fd, c] : server.clients)
		{
			(void)fd;
			if (are_equal_strs_case_insensitive(
					target.data(), target.size(),
					c.nick.data(), c.nick.size()))
			{
				target_client = &c;
				break;
			}
		}
		if (!target_client)
		{
			// TODO: build_ERR_NOSUCHNICK(client, target);
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
		// TODO: build_ERR_INVALIDCHANNELNAME(client, channel_name);
		return;
	}

	// Find existing channel, or create a new one
	auto			ch_it = server.channels.find(channel_name);
	if (ch_it == server.channels.end())
	{
		if (server.channels.size() >= MAX_CHANNELS)
		{
			// TODO: build_ERR_TOOMANYCHANNELS(client, channel_name);
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

	// Reject if LIMIT flag and at maximum channels per client
	if (is_flag_set(channel.mode, LIMIT) && channel.members.size() >= static_cast<size_t>(channel.user_limit))
   	{
		// TODO: build_ERR_CHANNELISFULL` (471 — channel at user limit).
		return;
	}
	// invite check
	if (is_flag_set(channel.mode, INVITE) && !channel.invited.contains(&client))
	{
		// TODO: build_ERR_INVITEONLYCHAN(client, channel_name);
		return;
	}
	// key check
	if (is_flag_set(channel.mode, KEY))
	{
		if (client.parser.n_params < 2 || channel.key != client.parser.params[1])
		{
			// TODO: build_ERR_BADCHANNELKEY(client, channel_name);
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

	// TODO: build_RPL_JOIN(client, channel_name);
}

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		channel_name(client.parser.params[0]);
	auto			channel_it = server.channels.find(channel_name);
	if (channel_it == server.channels.end())
	{
		//TODO: build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}
	t_IRC_Channel	&channel = channel_it->second;

	auto			it = channel.members.find(&client);
	if (it == channel.members.end())
	{
		//TODO: build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}

	// Record membership on channel and client
	client.joined_channels.erase(&channel);
	channel.members.erase(it);
	if (channel.members.empty())
		server.channels.erase(channel_it);
	// TODO: build_RPL_PART(client, channel_name);
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
	auto			channel_it = server.channels.find(channel_name);

	if (channel_it == server.channels.end())
	{
		// TODO: build_ERR_NOSUCHCHANNEL(kicker, channel_name);
		return;
	}
	t_IRC_Channel	&channel = channel_it->second;

	auto			kicker_it = channel.members.find(&kicker);
	if (kicker_it == channel.members.end())
	{
		// TODO: build_ERR_NOTONCHANNEL(kicker, channel_name);
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		// TODO: build_ERR_CHANOPRIVSNEEDED(kicker, channel_name);
		return;
	}

	t_IRC_Client	*to_be_kicked = find_chmember_by_nick(channel, victim);
	if (!to_be_kicked)
	{
		// TODO: build_ERR_USERNOTINCHANNEL(kicker, victim, channel_name);
		return;
	}

	// Record membership on channel and client
	to_be_kicked->joined_channels.erase(&channel);
	channel.members.erase(to_be_kicked);
	if (channel.members.empty())
		server.channels.erase(channel_it);
	// TODO: build_RPL_KICK(client, channel_name, victim);
}

void	execute_MODE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		target_nick(client.parser.params[0]);
	std::string		channel_name(client.parser.params[1]);
	auto			channel_it = server.channels.find(channel_name);
	if (channel_it == server.channels.end())
	{
		// TODO: build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}
	t_IRC_Channel	&channel = channel_it->second;

	if (!channel.members.contains(&client))
	{
		// TODO: build_ERR_CHANOPRIVSNEEDED(client, channel_name);
		return;
	}
	//TODO: build_RPL_UMODEIS with numeric replies. Needed pretty soon for irssi to work.
	//no params->print current mode
	if (client.parser.n_params == 1)
	{
		// TODO: build_RPL_CHANNELMODEIS(client, channel_name, mode(needs to be parsed));
		return;
	}
	//TODO: this needs to be a loop that goes trough all the parameters
	// with + toggling set and - unset.
	if (client.parser.n_params > 1 && is_flag_set(channel.members[&client], IS_OPERATOR)) //TODO: do this with iterator. [] makes a new item if not found
	{
		if (client.parser.params[1] == "+i")
		{
			channel.mode |= INVITE;
			// TODO: build_RPL_CHANNELMODEIS(client, channel_name, channel.mode);
		}
		if (client.parser.params[1] == "+k")
		{
			channel.mode |= KEY;
			// TODO: build_RPL_CHANNELMODEIS(client, channel_name, channel.mode);
		}
		if (client.parser.params[1] == "+t")
		{
			channel.mode |= TOPIC;
			// TODO: build_RPL_CHANNELMODEIS(client, channel_name, channel.mode);
		}
		if (client.parser.params[1] == "+l")
		{
			channel.mode |= LIMIT;
			// TODO: build_RPL_CHANNELMODEIS(client, channel_name, channel.mode);
		}
		if (client.parser.params[1] == "+o")
			channel.members[&client] |= IS_OPERATOR;
		// else if (client.parser.params[1] == "-o")
		// 	channel.members[&client] &= ~IS_OPERATOR;
		else
		{
			// TODO: build_ERR_CHANOPRIVSNEEDED(client, channel_name);
			return;
		}
	}
}
