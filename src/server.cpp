#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <string>
#include <sys/poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <vector>
#include <iostream>
#include "../lib/irc_fatstruct.hpp"

void	fatal_server_error(const char* msg, int fd)
{
	std::perror(msg);
	if (fd>=0)
		close(fd);
	exit(1);
}

void	setup_listener(t_IRC_Server &server)
{
	server.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.listen_fd < 0)
		fatal_server_error("socket", -1);

	int	opt = 1;
	if (setsockopt(server.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		fatal_server_error("setsockopt", server.listen_fd);

	sockaddr_in	socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_addr.sin_port = htons(server.port);

	if (bind(server.listen_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
		fatal_server_error("bind", server.listen_fd);

	if (listen(server.listen_fd, MAX_PENDING_CONNECTIONS) < 0)
		fatal_server_error("listen", server.listen_fd);

	std::cout << "Listening on port " << server.port << "\n";
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	std::cout << "Client disconnected.\n";
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
}

bool	setup_client(t_IRC_Server &server, int client_fd, struct sockaddr_in const &client_addr)
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

bool	handle_poll_event(t_IRC_Server &server, int fd, short rev)
{
	if (rev & (POLLERR | POLLHUP | POLLNVAL))
	{
		if (fd == server.listen_fd)
			fatal_server_error("listen socket poll event", -1);
		disconnect_client(server, fd);
		return true;
	}

	if (rev & POLLIN)
	{
		if (fd == server.listen_fd)
		{
			accept_new_client(server);
			return false;
		}
		if (recv_from_client(server, fd))
		{
			disconnect_client(server, fd);
			return true;
		}
		handle_client_message(server.clients[fd]);
	}
		return false;
}

void	server_loop(t_IRC_Server &server)
{
	server.poll_fds.push_back(pollfd{server.listen_fd, POLLIN, 0});

	while (1)
	{
		if (poll(server.poll_fds.data(), static_cast<nfds_t>(server.poll_fds.size()), 0) < 0)
		{
			if (errno == EINTR)
				continue;
			fatal_server_error("poll", -1);
		}

		for (size_t i = 0; i < server.poll_fds.size(); )
		{
			int		fd = server.poll_fds[i].fd;
			short	rev = server.poll_fds[i].revents;
			bool	is_removed = handle_poll_event(server, fd, rev);
			if (!is_removed)
				i++;
		}
	}
}
