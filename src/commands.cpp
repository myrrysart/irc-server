#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include "../lib/channel.hpp"
#include <cstring> // for std::strncmp()
#include <string_view>

void	execute_PING_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NOORIGIN(client);
		return ;
	}
	client.send_message_buffer += ":";
	client.send_message_buffer += server.name;
	client.send_message_buffer += " PONG ";
	client.send_message_buffer += server.name;
	client.send_message_buffer += " :";
	client.send_message_buffer += client.parser.params[0];
	client.send_message_buffer += "\r\n";
}

void	execute_PONG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	(void) server;
	(void) client;
}

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
			default: build_ERR_UNKNOWNCOMMAND(client);     break;
			case 0:  execute_PASS_cmd(client, server);     break;
			case 1:  execute_NICK_cmd(client, server);     break;
			case 2:  execute_USER_cmd(client);             break;
			case 3:  execute_QUIT_cmd(client, server);     break;
			case 4:  execute_JOIN_cmd(client, server);     break;
			case 5:  execute_PART_cmd(client, server);     break;
			case 6:  execute_PRIVMSG_cmd(client, server);  break;
			case 7:  execute_MODE_cmd(client, server);     break;
			case 8:  execute_KICK_cmd(client, server);     break;
			case 9:  execute_INVITE_cmd(client, server);   break;
			case 10: execute_TOPIC_cmd(client, server);    break;
			case 11: execute_NAMES_cmd(client, server);    break;
			case 12: execute_LIST_cmd(client, server);     break;
			case 13: execute_PING_cmd(client, server);     break;
			case 14: execute_PONG_cmd(client, server);     break;
		}
	}

	client.parser.n_params = 0;
}

/* message string builder for execute_QUIT_cmd() */
static void	build_quit_message(std::string &quit_msg, t_IRC_Client &quitter)
{
	const std::string_view	middle_part{" QUIT :Quit: "};

	// calculate how long the message will be, to allow pre-reservation of
	// capacity, avoiding potential std::string reallocations
	size_t	len = quitter.nick.size() + quitter.username.size()
		+ sizeof(quitter.hostname) + middle_part.size()
		+ quitter.parser.params[0].size() + 5; // 5: ':' + '!' + '@' + '\r' + '\n'
	quit_msg.reserve(len);

	quit_msg += ':';
	append_nick_user_host(quit_msg, quitter);
	quit_msg += middle_part;

	// append the reason provided by the departing client
	// if no reason provided or reason is empty: the client should still receive
	// ":Quit: ", implemented in the string literal 'middle_part'
	if (quitter.parser.n_params)
		quit_msg += quitter.parser.params[0];
	quit_msg += "\r\n";
}

void    execute_QUIT_cmd(t_IRC_Client &quitter, t_IRC_Server &server)
{
	// According to the Modern IRC protocol: "The server acknowledges" the
	// QUIT command "by replying with an ERROR message"
	append_error_msg_quit(quitter, server.name);

	// trigger disconnection of client without alerting anyone,
	// if client is unregistered or not on any channel
	if (!is_flag_set(quitter.state, quitter.REGISTERED)
		|| quitter.joined_channels.empty())
	{
		quitter.state |= t_IRC_Client::DISCONNECT;
		return;
	}

	// build message to be broadcasted. Each client that shares a channel with
	// the departing client should be notified
	std::string	quit_msg;
	build_quit_message(quit_msg, quitter);

	broadcast_to_fellow_channelers_once_per_client(quitter, quit_msg);
	quitter.state |= t_IRC_Client::DISCONNECT;
}
