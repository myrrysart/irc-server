#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/parser.hpp"

#include <iostream>
#include <cstring> // for std::strncmp()
#include <string_view>

// TODO: Add prefix?

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
			if (!std::strncmp(t_parser::commands[i], verb_in_caps, verb_len))
				break ;
		}
	}
	else
		i = t_parser::n_valid_cmds;


	// WARN: Make 100% sure that the commands here match the ones in the commands array;
	// And also, make sure that all of those commands are implemented / need to be implemented!

	if (!is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		// Registration required - or unfinished
		client_registration(client, i, server);
	}
	else
	{
		// dispatch for all commands
		// TODO:
		switch (i)
		{
			default: invalid_command_detected(client); break;
			case 0:  execute_PASS_cmd(client, server); break;
			case 1:  execute_NICK_cmd(client, server); break;
			case 2:  execute_USER_cmd(client, server); break;
			// case 3:  execute_QUIT_cmd(client);         break;
			// case 4:  execute_JOIN_cmd(client);         break;
			// case 5:  execute_PART_cmd(client);      break;
			// case 6:  execute_PRIVMSG_cmd(client);         break;
			// case 7:  execute_MODE_cmd(client);         break;
			// case 8:  execute_KICK_cmd(client);       break;
			// case 9:  execute_INVITE_cmd(client);        break;
			// case 10: execute_TOPIC_cmd(client);         break;
			// case 11: execute_PING_cmd(client);         break;
			// case 12: execute_PONG_cmd(client);         break;
			// case 13: execute_NAMES_cmd(client);        break;
			// case 14: execute_LIST_cmd(client);         break;
		}
	}

	client.parser.n_params = 0;
}

// NOTE: When a messsage is unknown: Irssi (at least) returns:
// "Irssi: Unknown command: <the_command>", keeping the user's provided case.
// ----> do not output 'the_command' with a different case, remain case sensitive.
void	invalid_command_detected(const t_IRC_Client &client)
{
	// FIXME: send instead of printing.
	std::cout << "Unknown command: " << client.parser.verb << "\r\n";
}
