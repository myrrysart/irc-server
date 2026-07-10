#include "../lib/channel.hpp"
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
// NOTE: this is pretty much the same function to the one in main.cpp.
// Could both be turned to one generic helper?
static bool	parse_channel_limit(std::string_view limit, size_t &parsed_limit)
{
	const char				*begin = limit.data();
	const char				*end = begin + limit.size();
	std::from_chars_result	result = std::from_chars(begin, end, parsed_limit);

	if (result.ec != std::errc{} || result.ptr != end || parsed_limit == 0)
		return false;
	return true;
}

void	execute_MODE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	std::string_view	channel_name(client.parser.params[0]);
	// first param isn't a channel: irssi's auto `MODE <yournick>` on connect
	if (channel_name.empty() || (channel_name[0] != '#' && channel_name[0] != '&'))
	{
		trim_nickname_if_longer_than_max_nicklen(channel_name);

		//irssi auto-sends `MODE <yournick>` on connect
		if (are_equal_strs_case_insensitive(channel_name, client.nick))
		{
			if (client.parser.n_params > 1)
				build_ERR_UMODEUNKNOWNFLAG(client); // 501
			else
				build_RPL_UMODEIS(client); // 221
		}
		else
		{
			if (!find_client_by_nick(server, channel_name))
				build_ERR_NOSUCHNICK(client, channel_name); // 401
			else
				build_ERR_USERSDONTMATCH(client); // 502
		}
		return;
	}

	// no 2nd param -> report current modes
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
		return;
	}
	if (client.parser.n_params == 1)
	{
		build_RPL_CHANNELMODEIS(client, *channel); // 324
		return;
	}
	// changing modes checks for membership + operator
	auto	member_it = channel->members.find(&client);
	if (member_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel->name); // 442
		return;
	}
	if (!is_flag_set(member_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel->name); // 482
		return;
	}

	std::string_view	modes = client.parser.params[1];
	size_t				arg_idx = 2;	// next mode-argument param to consume (for k/l/o)
	char				sign = 0;
	size_t				i = 0;

	// Accumulate only applied changes for the broadcast
	// Each sign has its own bucket
	std::string			plus_chars;
	std::string			plus_args;
	std::string			minus_chars;
	std::string			minus_args;

	// walk the mode string char by char, tracking the current +/- sign
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
		// channel key. +k with argument, -k without
		else if (current_char == 'k')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					std::string_view	key = client.parser.params[arg_idx];
					arg_idx++;
					if (key.empty())
						continue;
					channel->mode |= KEY;
					channel->key.assign(key);
					plus_chars += 'k';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += key;
				}
				else
					continue ; // ignore request if required argument is not there
			}
			else
			{
				channel->mode &= ~KEY;
				channel->key.clear();
				minus_chars += 'k';
			}
		}
		// user limit. +l w. argument (parses numeric), -l no argument
		else if (current_char == 'l')
		{
			if (sign == '+')
			{
				if (arg_idx < client.parser.n_params)
				{
					std::string_view	limit = client.parser.params[arg_idx];
					arg_idx++;

					size_t				parsed_limit = 0;
					if (!parse_channel_limit(limit, parsed_limit))
						continue;

					channel->mode |= LIMIT;
					channel->user_limit = parsed_limit;
					plus_chars += 'l';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += limit;
				}
				else
					continue ; // required argument is not there -> ignore
			}
			else
			{
				channel->mode &= ~LIMIT;
				channel->user_limit = 0;
				minus_chars += 'l';
			}
		}
		// operator status on a target member. Always with an argument
		else if (current_char == 'o')
		{
			if (arg_idx < client.parser.n_params)
			{
				std::string_view	target_nick = client.parser.params[arg_idx];
				trim_nickname_if_longer_than_max_nicklen(target_nick);
				t_IRC_Client		*target =
					find_chmember_by_nick(*channel, target_nick);
				arg_idx++;
				if (!target)
					build_ERR_USERNOTINCHANNEL(client, channel->name, target_nick); // 441
				else if (sign == '+')
				{
					channel->members[target] |= IS_OPERATOR;
					plus_chars += 'o';
					if (!plus_args.empty())
						plus_args += ' ';
					plus_args += target->nick;
				}
				else
				{
					channel->members[target] &= ~IS_OPERATOR;
					minus_chars += 'o';
					if (!minus_args.empty())
						minus_args += ' ';
					minus_args += target->nick;
				}
			}
			else
				continue ; // required argument ain't there -> ignore request
		}
		else
			build_ERR_UNKNOWNMODE(client, current_char); // 472
	}

	// broadcast only the changes that actually applied, e.g. "+ik key -l"
	std::string	applied_mode_changes;
	append_sign_group(applied_mode_changes, '+', plus_chars,  plus_args);
	append_sign_group(applied_mode_changes, '-', minus_chars, minus_args);

	if (!applied_mode_changes.empty())
	{
		std::string	line;
		append_MODE_msg(line, client, channel->name, applied_mode_changes);
		broadcast_to_channel(*channel, line, client, false);
	}
}
