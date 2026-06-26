#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include "../lib/channel.hpp"
#include <cstring> // for std::strncmp()
#include <string_view>

void	execute_PING_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	client.send_message_buffer += ":";
	client.send_message_buffer += server.name;
	client.send_message_buffer += " PONG ";
	client.send_message_buffer += server.name;
	client.send_message_buffer += " :";
	client.send_message_buffer += server.name;
	client.send_message_buffer += "\r\n";
}

void	execute_PONG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	(void) server;
	(void) client;
}
// WARN: To avoid bad invalid memory access surprises: Once PRIVMSG is implemented,
// Test something like: "PRIVMSG :some message" followed by "PRIVMSS :whatever".

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

	// WARN: Make 100% sure that the commands in the switch case match the ones
	// in the commands array; Also, make sure that all of those commands are
	// implemented / need to be implemented!

	if (!is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		// Registration required - or unfinished
		client_registration(client, i, server);
	}
	else
	{
		// dispatch for all commands
		// TODO: Work in progress.
		switch (i)
		{
			default: build_ERR_UNKNOWNCOMMAND(client);		break;
			case 0:  execute_PASS_cmd(client, server);		break;
			case 1:  execute_NICK_cmd(client, server);		break;
			case 2:  execute_USER_cmd(client);				break;
			// case 3:  execute_QUIT_cmd(client, server);		break;
			case 4:  execute_JOIN_cmd(client, server);		break;
			case 5:  execute_PART_cmd(client, server);		break;
			case 6:  execute_PRIVMSG_cmd(client, server);	break;
			case 7:  execute_MODE_cmd(client, server);		break;
			case 8:  execute_KICK_cmd(client, server);		break;
			// case 9:  execute_INVITE_cmd(client, server);	break;
			case 10: execute_TOPIC_cmd(client, server); 	break;
			case 13: execute_NAMES_cmd(client, server); 	break; // all nicks on channel
			// case 14: execute_LIST_cmd(client, server);  	break; // all channels on server
			case 11: execute_PING_cmd(client, server);		break;
			case 12: execute_PONG_cmd(client, server);		break;
		}
	}

	client.parser.n_params = 0;
}

// WARN: work in progress
// void    execute_QUIT_cmd(t_IRC_Client &client, t_IRC_Server &server)
// {
//     if (is_flag_set(client.state, t_IRC_Client::REGISTERED) &&
//         !client.joined_channels.empty())
//     {
//         // start building the message to be sent - do it only once so that it
//         // is easy to be appended to multiple clients.
//         static constexpr std::string_view    middle_part{" QUIT :Quit: "};  // this is a string literal

//         size_t    len = client.nick.size() + client.username.size() +
//             sizeof(client.hostname) + middle_part.size() +
//             client.parser.params[0].size() + 4; // 4: '!' + '@' + '\r' + '\n'

//         std::string    quit_msg;
//         quit_msg.reserve(len);
//         append_nick_user_host(quit_msg, client);
//         quit_msg += " QUIT :Quit: ";
//         if (client.parser.n_params)
//             quit_msg += client.parser.params[0];
//         quit_msg += "\r\n";

//         // temporary placeholder definition for this struct, if this is kept, it would go elsewhere, of course:
//         // this idea for this struct is to allow keeping track of the clients that have already gotten the message
//         // appended to their output buffer, so that they would not end up getting duplicate messages just because they share
//         // more than one channel with the quitting client.
//         // This may be specific for QUIT command, however.
//         struct    s_already_sent
//         {
//            // MAX_CLIENTS can/should be replaced with whatever maximal value of clients per channel we have.
//            // or maybe there is a better way to achieve that, since the unordered_set of channel members has a size?
//            // but maybe using that would be a VLA (variable length array), which is always a bad idea.
//             int        fds[MAX_CLIENTS];
//               size_t    count;
//         };

//            // init that struct, careful: fds contains garbage values, always tread with 'count' size.
//            static    s_already_sent    already_sent;
//            already_sent.count = 0;

//            // iterate through all channels the client is connected to.
//            for (std::unordered_set<t_IRC_Channel>::const_iterator channel_it = client.joined_channels.begin(); channel_it != client.joined_channels.end(); ++channel_it)
//            {
//                // iterate through all members connected to the current channel. Syntax can of course be replaced by 'auto'
//                for (std::unordered_set<t_IRC_ChannelMembership>::const_iterator member_it = channel_it.members.begin(); member_it != channel_it.members.end(); ++member_it;)
//                {
//                    // no need to send the message to itself
//                    // nor to a member that already has the message ready in its buffer
//                    if (member_it.fd == client.fd || has_received_message(already_sent, member_it.fd))
//                        continue;
//                    member_it.send_message_buffer += quit_msg;
//                            // can be replaced with whatever appending function (something like 'build_msg_to_send(member_it.send_message_buffer));'
//                    already_sent.fds[already_sent.count] = member_it.fd;
//                    ++(already_sent.count);
//                }
//            }
//        }
//        // set the disconnect flag (this is for QUIT, of course)
//        client.state |= t_IRC_Client::DISCONNECT;
//    }
