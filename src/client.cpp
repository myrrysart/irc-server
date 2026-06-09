#include <iostream>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char	buf[t_parser::buf_size];
	t_IRC_Client	&client = server.clients[fd]; // reference to the observed client

	if (is_flag_set(client.state, t_IRC_Client::ERROR))
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
		prepare_and_parse_message(pos, buf, client);
		dispatch_client_command(client, server);

		// WARN: The next statement should be reviewed once the queue system is
		// in place: Sending error message/s to the client might be required
		// BEFORE disconnecting it from the server.
		if (is_flag_set(client.state, t_IRC_Client::ERROR))
			return true;

		buf.erase(0, pos + 1);
	}

	check_for_too_long_message(buf, client);
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
