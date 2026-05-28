#include <iostream>
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"

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
	int			client_fd = accept(server.listen_fd, (sockaddr*)&client_addr, &client_addr_len);
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
	std::cout << "new client connected" << std::endl;
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}

static bool	handle_poll_event(t_IRC_Server &server, int fd, short rev)
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
	std::cout << "server running at port " << server.port << std::endl;

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
