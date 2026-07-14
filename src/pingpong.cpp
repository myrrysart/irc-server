#include "../lib/irc_fatstruct.hpp"
#include "../lib/numerics.hpp"

#include <string>

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
