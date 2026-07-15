#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include "../lib/channel.hpp"

#include <cstring> // for std::strncmp()
#include <string_view>

void	dispatch_client_command(t_IRC_Client &client, t_IRC_Server &server)
{
	char	*verb_in_caps = client.parser.verb_in_caps;
	size_t	verb_len = client.parser.verb.size();
	size_t	i;

	if (verb_len <= t_parser::longest_cmd_size)
	{
		for (size_t j = 0; j < verb_len; ++j)
			verb_in_caps[j] = to_uppercase(client.parser.verb[j]);

		for (i = 0; i < t_parser::n_valid_cmds; ++i)
		{
			if (verb_len == t_parser::commands[i].size() &&
					!std::strncmp(t_parser::commands[i].data(), verb_in_caps, verb_len))
				break ;
		}
	}
	else
		i = t_parser::n_valid_cmds; // invalid command, will trigger default case

	if (!is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		// Registration required - or unfinished
		client_registration(client, i, server);
	}
	else
	{
		// dispatch for all commands
		switch (i)
		{
			default:  build_ERR_UNKNOWNCOMMAND(client);     break;
			case 0:   execute_CAP_cmd(client);              break;
			case 1:   execute_PASS_cmd(client, server);     break;
			case 2:   execute_NICK_cmd(client, server);     break;
			case 3:   execute_USER_cmd(client);             break;
			case 4:   execute_QUIT_cmd(client, server);     break;
			case 5:   execute_JOIN_cmd(client, server);     break;
			case 6:   execute_PART_cmd(client, server);     break;
			case 7:   execute_PRIVMSG_cmd(client, server);  break;
			case 8:   execute_MODE_cmd(client, server);     break;
			case 9:   execute_KICK_cmd(client, server);     break;
			case 10:  execute_INVITE_cmd(client, server);   break;
			case 11:  execute_TOPIC_cmd(client, server);    break;
			case 12:  execute_NAMES_cmd(client, server);    break;
			case 13:  execute_LIST_cmd(client, server);     break;
			case 14:  execute_PING_cmd(client, server);     break;
			case 15:  execute_PONG_cmd(client, server);     break;
		}
	}

	client.parser.n_params = 0;
}
