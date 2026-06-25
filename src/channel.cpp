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
	if (auto name_it = server.channels.find(ch_name); name_it != server.channels.end())
		return &name_it->second;
	return nullptr;
}

static void	remove_client_from_channel(t_IRC_Client &client, t_IRC_Channel &channel, t_IRC_Server &server)
{
	client.joined_channels.erase(&channel);
	channel.members.erase(&client);
	channel.invited.erase(&client);
	if (channel.members.empty())
		server.channels.erase(channel.name);
}

static t_IRC_Client	*find_client_by_nick(t_IRC_Server &server, const std::string_view nick)
{
	for (auto &[fd, client] : server.clients)
	{
		(void)fd;
		if (are_equal_strs_case_insensitive(
				nick.data(), nick.size(),
				client.nick.data(), client.nick.size()))
			return &client;
	}
	return nullptr;
}

static void	broadcast_to_channel(t_IRC_Channel &channel, const std::string &line)
{
	for (auto &[member_ptr, flags] : channel.members)
		member_ptr->send_message_buffer += line;
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

		for (auto &[member_ptr, flags] : channel->members)
		{
			if (member_ptr != &client)
				member_ptr->send_message_buffer += line;
		}
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

	std::string		line = ":";
	append_nick_user_host(line, client);
	line += " JOIN ";
	line += channel_name;
	line += "\r\n";
	broadcast_to_channel(channel, line);
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

	std::string		line = ":";
	append_nick_user_host(line, client);
	line += " PART ";
	line += channel_name;
	if (client.parser.n_params >= 2)
	{
		line += " :";
		line += client.parser.params[1];
	}
	line += "\r\n";
	broadcast_to_channel(*channel, line);
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

	std::string		line = ":";
	append_nick_user_host(line, kicker);
	line += " KICK ";
	line += channel_name;
	line += ' ';
	line += to_be_kicked->nick;
	if (kicker.parser.n_params >= 3)
	{
		line += " :";
		line += kicker.parser.params[2];
	}
	line += "\r\n";
	broadcast_to_channel(*channel, line);
	remove_client_from_channel(*to_be_kicked, *channel, server);
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
		build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}
	auto	member_it = channel->members.find(&client);
	if (member_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}
	if (client.parser.n_params == 1)
	{
		build_RPL_CHANNELMODEIS(client, *channel);
		return;
	}
	if (!is_flag_set(member_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel_name);
		return;
	}

	std::string_view	modes = client.parser.params[1];
	size_t				arg_idx = 2;
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
		if (!sign)  // skip invalid
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
				else build_ERR_NEEDMOREPARAMS(client);
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
				else build_ERR_NEEDMOREPARAMS(client);
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
				std::string_view	target_nick = client.parser.params[arg_idx];
				t_IRC_Client		*target =
					find_chmember_by_nick(*channel, target_nick);
				arg_idx++;
				if (!target)
					build_ERR_USERNOTINCHANNEL(client, channel_name, target_nick);
				else if (sign == '+')
					channel->members[target] |= IS_OPERATOR;
				else
					channel->members[target] &= ~IS_OPERATOR;
			}
			else build_ERR_NEEDMOREPARAMS(client);
		}
		else
			build_ERR_UNKNOWNMODE(client, channel_name, current_char);
	}
}
