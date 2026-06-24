#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, const std::string_view nick)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (are_equal_strs_case_insensitive(nick.data(), nick.size(), member_ptr->nick.data(), member_ptr->nick.size()))
			return member_ptr;
	}
	return nullptr;
}

t_IRC_Channel	*find_channel_by_name(t_IRC_Server &server, const std::string &ch_name)
{
	if (auto it = server.channels.find(ch_name); it != server.channels.end())
		return &it->second;
	return nullptr;
}

static void	remove_client_from_channel(t_IRC_Client &client, t_IRC_Channel &channel, t_IRC_Server &server)
{
	client.joined_channels.erase(&channel);
	channel.members.erase(&client);
	if (channel.members.empty())
		server.channels.erase(channel.name);
}

static t_IRC_Client	*find_client_by_nick(t_IRC_Server &server, const std::string_view nick)
{
	for (auto &[fd, c] : server.clients)
	{
		(void)fd;
		if (are_equal_strs_case_insensitive(
				nick.data(), nick.size(),
				c.nick.data(), c.nick.size()))
			return &c;
	}
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

	std::string		line = ":";
	append_nick_user_host(line, client);
	line += " PRIVMSG ";

	if (target[0] == '#' || target[0] == '&')
	{
		t_IRC_Channel	*channel = find_channel_by_name(server, target);
		if (!channel)
		{
			// TODO: build_ERR_NOSUCHCHANNEL(client, target);
			return;
		}

		if (!channel->members.contains(&client))
		{
			// TODO: build_ERR_CANNOTSENDTOCHAN(client, target);
			return;
		}

		line += channel->name;
		line += " :";
		line += message;
		line += "\r\n";

		for (auto &[member_ptr, flags] : channel->members)
			member_ptr->send_message_buffer += line;
	}
	else
	{
		t_IRC_Client	*target_client = find_client_by_nick(server, target);
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

	// limit check
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

	t_IRC_Channel	*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		//TODO: build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}

	if (!channel->members.contains(&client))
	{
		//TODO: build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}

	remove_client_from_channel(client, *channel, server);
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

	t_IRC_Channel	*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		// TODO: build_ERR_NOSUCHCHANNEL(kicker, channel_name);
		return;
	}

	auto			kicker_it = channel->members.find(&kicker);
	if (kicker_it == channel->members.end())
	{
		// TODO: build_ERR_NOTONCHANNEL(kicker, channel_name);
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		// TODO: build_ERR_CHANOPRIVSNEEDED(kicker, channel_name);
		return;
	}

	t_IRC_Client	*to_be_kicked = find_chmember_by_nick(*channel, victim);
	if (!to_be_kicked)
	{
		// TODO: build_ERR_USERNOTINCHANNEL(kicker, victim, channel_name);
		return;
	}

	remove_client_from_channel(*to_be_kicked, *channel, server);
	// TODO: build_RPL_KICK(client, channel_name, victim);
}

void	execute_MODE_cmd(t_IRC_Client &client, t_IRC_Server &server)
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
		// TODO: build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}
	// on channel check
	auto	member_it = channel->members.find(&client);
	if (member_it == channel->members.end())
	{
		// TODO: build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}

	if (client.parser.n_params == 1)
	{
		// TODO: build_RPL_CHANNELMODEIS(client, channel_name, channel->mode);
		return;
	}

	// check operator status
	if (!is_flag_set(member_it->second, IS_OPERATOR))
	{
		// TODO: build_ERR_CHANOPRIVSNEEDED(client, channel_name);
		return;
	}

	std::string_view	modes = client.parser.params[1];
	size_t				arg_idx = 2; //read from param[2] onwards
	char				sign = 0;
	size_t				i = 0;

	while (i < modes.size())
	{
		char	current_char = modes[i];
		i++;

		if (current_char == '+' || current_char == '-')
		{
			sign = current_char;
			continue;
		}
		if (!sign)  // skip non-sign
			continue;

		if (current_char == 'i')
		{
			if (sign == '+')
				channel->mode |= INVITE;
			else
				channel->mode &= ~INVITE;
		}
		else if (current_char == 't')
		{
			if (sign == '+')
				channel->mode |= TOPIC;
			else
				channel->mode &= ~TOPIC;
		}
		else if (current_char == 'k')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					channel->mode |= KEY;
					channel->key = std::string(client.parser.params[arg_idx]);
					arg_idx++;
				}
				// else: TODO: build_ERR_NEEDMOREPARAMS(client);
			}
			else
			{
				channel->mode &= ~KEY;
				channel->key.clear();
			}
		}
		else if (current_char == 'l')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					channel->mode |= LIMIT;
					channel->user_limit =
						std::atoi(std::string(client.parser.params[arg_idx]).c_str());
					arg_idx++;
				}
				// else: TODO: build_ERR_NEEDMOREPARAMS(client);
			}
			else
			{
				channel->mode &= ~LIMIT;
			}
		}
		else if (current_char == 'o')
		{
			if (arg_idx < client.parser.n_params)
			{
				t_IRC_Client	*target =
					find_chmember_by_nick(*channel, client.parser.params[arg_idx]);
				arg_idx++;
				if (!target)
				{
					// TODO: build_ERR_USERNOTINCHANNEL(client, nick, channel_name);
				}
				else if (sign == '+')
					channel->members[target] |= IS_OPERATOR;
				else
					channel->members[target] &= ~IS_OPERATOR;
			}
			// else: TODO: build_ERR_NEEDMOREPARAMS(client);
		}
		else
		{
			// TODO: build_ERR_UNKNOWNMODE(client, current_char);
		}
	}
}
