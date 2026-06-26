#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <charconv>
#include <string>

static void	append_sign_group(std::string &out, char sign,
		const std::string &chars, const std::string &args)
{
	if (chars.empty())
		return;
	if (!out.empty())
		out += ' ';
	out += sign;
	out += chars;
	if (!args.empty())
	{
		out += ' ';
		out += args;
	}
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

	// Accumulate only applied changes for the broadcast
	// Each sign has its own bucket
	std::string			plus_chars;
	std::string			plus_args;
	std::string			minus_chars;
	std::string			minus_args;

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
			{
				channel->mode |= INVITE;
				plus_chars += 'i';
			}
			else
			{
				channel->mode &= ~INVITE;
				minus_chars += 'i';
			}
		}
		else if (current_char == 't')
		{
			if (sign == '+')
			{
				channel->mode |= TOPIC;
				plus_chars += 't';
			}
			else
			{
				channel->mode &= ~TOPIC;
				minus_chars += 't';
			}
		}
		else if (current_char == 'k')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					channel->mode |= KEY;
					channel->key.assign(client.parser.params[arg_idx]);
					plus_chars += 'k';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += client.parser.params[arg_idx];
					arg_idx++;
				}
				else build_ERR_NEEDMOREPARAMS(client);
			}
			else
			{
				channel->mode &= ~KEY;
				channel->key.clear();
				minus_chars += 'k';
			}
		}
		else if (current_char == 'l')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					std::string_view	limit = client.parser.params[arg_idx];

					channel->mode |= LIMIT;
					channel->user_limit = 0;
					std::from_chars(limit.data(), limit.data() + limit.size(), channel->user_limit);
					plus_chars += 'l';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += limit;
					arg_idx++;
				}
				else build_ERR_NEEDMOREPARAMS(client);
			}
			else
			{
				channel->mode &= ~LIMIT;
				minus_chars += 'l';
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
				{
					channel->members[target] |= IS_OPERATOR;
					plus_chars += 'o';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += target_nick;
				}
				else
				{
					channel->members[target] &= ~IS_OPERATOR;
					minus_chars += 'o';
					if (!minus_args.empty())
						minus_args += ' ';
					minus_args += target_nick;
				}
			}
			else build_ERR_NEEDMOREPARAMS(client);
		}
		else
			build_ERR_UNKNOWNMODE(client, channel_name, current_char);
	}

	std::string	delta;
	append_sign_group(delta, '+', plus_chars,  plus_args);
	append_sign_group(delta, '-', minus_chars, minus_args);

	if (!delta.empty())
	{
		std::string	line;
		append_MODE_msg(line, client, channel_name, delta);
		broadcast_to_channel(*channel, line, client, false);
	}
}
