#include <iostream>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	char	buf[512];

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received <= 0)
		return true;
	server.clients[fd].received_message_buffer.append(buf, received);
	return false;
}

void	handle_client_message(t_IRC_Client &client)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos;

	while((pos = buf.find('\n')) != std::string::npos)
	{
		std::cout	<< "Received from " << client.fd << " : "
					<< buf.substr(0, pos) << std::endl;
		buf.erase(0, pos + 1);

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
