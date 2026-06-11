#include <iostream>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"
#include "../lib/commands.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char		buf[t_parser::buf_size];
	t_IRC_Client	&client = server.clients[fd]; // reference to the observed client

	if (is_flag_set(client.state, t_IRC_Client::DISCONNECT))
		return true;

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received <= 0)
		return true;

	if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG))
		handle_message_to_discard(client, buf, received);
	else
		client.received_message_buffer.append(buf, received);

	return false;
}

bool	handle_client_message(t_IRC_Client &client, t_IRC_Server &server)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		if (prepare_and_parse_message(pos, buf, client) == -1)
		{
		// WARN: Shouldn't messages to the client that is about to be disconnected
		// still be sent to them beforehand?
			client.state |= client.DISCONNECT;
			server.state &= ~SERVER_RUNNING;
			return true;
		}
		if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG))
			client.state &= ~t_IRC_Client::DISCARD_MSG;
		else
		{
			dispatch_client_command(client, server);

			// TODO: send messages here?


		}
		buf.erase(0, pos + 1);
	}

	if (check_for_too_long_message(buf, client) == -1)
	{
		// WARN: Shouldn't messages to the client that is about to be disconnected
		// still be sent to them beforehand?
		// WARN: Consider adding a flushing loop for sending messages to the all
		// clients in the shutdown_server() function? And perhaps add a flag for
		// fatal hardware issues, that would communicate that sending more messages
		// is perhaps unnecessary in that extreme case?
		client.state |= client.DISCONNECT;
		server.state &= ~SERVER_RUNNING;
		return true;
	}
	return false;
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
	std::cout << "Client disconnected.\n";
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}
