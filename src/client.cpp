#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include <iostream>

static bool	setup_client(t_IRC_Server &server, int client_fd, struct sockaddr_in const &client_addr)
{
	int	one = 1;
	if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) < 0)
	{
		std::perror("setsockopt");
		close(client_fd);
		return false;
	}
	server.poll_fds.push_back(pollfd{client_fd, POLLIN, 0});
	server.clients[client_fd] = t_IRC_Client{};
	server.clients[client_fd].fd = client_fd;
	server.clients[client_fd].addr = client_addr;
	return true;
}

void	accept_new_client(t_IRC_Server &server)
{
	sockaddr_in client_addr{};
	socklen_t	client_addr_len = sizeof(client_addr);
	int client_fd = accept(server.listen_fd, (sockaddr*)&client_addr, &client_addr_len);
	if (client_fd < 0)
	{
		std::perror("accept");
		return;
	}
	if (server.clients.size() >= MAX_CLIENTS)
	{
		std::cout << "This server is full." << std::endl;
		close(client_fd);
		return;
	}
	if (!setup_client(server, client_fd, client_addr))
		return;
}

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
	std::cout << "Client disconnected.\n";
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
}
