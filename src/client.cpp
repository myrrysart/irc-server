#include <iostream>
#include <exception>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char		buf[t_parser::buf_size];
	t_IRC_Client	&client = server.clients[fd]; // reference to the observed client

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received <= 0)
		return true;

	if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG))
		handle_message_to_discard(client, buf, received, server);
	else
		client.received_message_buffer.append(buf, received);

	return false;
}

void	handle_client_message(t_IRC_Client &client, t_IRC_Server &server)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos = buf.find('\n');

	if (pos != std::string::npos)
	{
		if (parse_message(pos, buf, client) == -1)
		{
			try {
				build_ERR_INPUTTOOLONG(client);
			} catch (const std::exception &e) {
				log_error(e.what(), __FILE__, __LINE__, 1);
				server.state &= ~SERVER_RUNNING;
				// WARN: Append error message to be sent to the client? Or this
				// might be too great of an error?
				return;
			}
		}

		if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG)) // no need to dispatch
			client.state &= ~t_IRC_Client::DISCARD_MSG;
		else
			dispatch_client_command(client, server);
		buf.erase(0, pos + 1);

	}
	else if (buf.length() >= t_parser::buf_size) // WARN: maybe this should be 'if', not 'else if'; but this might be good enough, since the buffer is cleared up from long input
	{
		try {
			build_ERR_INPUTTOOLONG(client);
		} catch (const std::exception &e) {
			log_error(e.what(), __FILE__, __LINE__, 1);
			// WARN: Append error message to be sent to the client? Or this
			// might be too great of an error?
			server.state &= ~SERVER_RUNNING;
			return;

		}
		client.state |= t_IRC_Client::DISCARD_MSG;
		buf.clear();
		/* NOTE: If user sends an extremely long buffer with no newlines:
		/ current behaviour would discard all of it, until they provide a '\n':
		/ Only after that '\n' it would start to process bytes, looking for a
		/ candidate message. */
	}
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
	std::cout << "Client disconnected.\n";
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}
