#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include "../lib/irc_fatstruct.hpp"

void  setup_socket(t_IRC_Server &server)
{
	 server.listen_fd  = socket(AF_INET, SOCK_STREAM, 0);
	if (server.listen_fd < 0)
	{
		std::perror("socket");
		exit(1);
	}

	int opt = 1;
	if (setsockopt(server.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::perror("setsockopt");
		close(server.listen_fd);
		exit(1);
	}

	sockaddr_in socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY); //  = any available interface
	socket_addr.sin_port = htons(server.port);

	if (bind(server.listen_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
	{
		std::perror("bind");
		close(server.listen_fd);
		exit(1);
	}

	if (listen(server.listen_fd, MAX_PENDING_CONNECTIONS) < 0)
	{
		std::perror("listen");
		close(server.listen_fd);
		exit(1);
	}

	std::cout << "Listening on port " << server.port << "\n";
}

void disconnect_client(t_IRC_Server &server, int fd)
{
	close(fd);
	server.clients.erase(fd);
	//TODO: remove the poll entry
}

void accept_new_client(t_IRC_Server &server)
{
	int client_fd = accept(server.listen_fd, nullptr, nullptr);
	if (client_fd < 0)
	{
		std::perror("accept");
		return;
	}
	server.poll_fds.push_back(pollfd{client_fd, POLLIN, 0});
	//TODO: insert to client map
}

bool read_from_client(t_IRC_Server &server, int fd)
{
	char buf[512];
	ssize_t received = recv(fd, buf, sizeof(buf), 0);
	if (received == 0)
	{
		std::cout << "Client disconnected.\n";
		disconnect_client(server, fd);
		return true;
	}
	if (received < 0)
	{
		std::perror("recv");
		disconnect_client(server, fd);
		return true;
	}
	if (received > 0)
	{
		server.clients[fd].received_message_buffer.append(buf, received);
		// if(terminator) parse and erase receive_message_buffer
	}
	return false;
}

void server_loop(t_IRC_Server &server)
{
	server.poll_fds.push_back(pollfd{server.listen_fd, POLLIN, 0});
	while (1)
	{
		if (poll(server.poll_fds.data(), static_cast<nfds_t>(server.poll_fds.size()), -1) < 0)
		{
			std::perror("poll");
			break;
		}

		size_t i = 0;
		while (i < server.poll_fds.size())
		{
			pollfd& poll_fd = server.poll_fds[i];
			int fd = poll_fd.fd;
			if (poll_fd.revents & POLLERR)
			{
				if (poll_fd.fd == server.listen_fd)
				{
					std::cerr << "Listen socket error.\n";
					return;
				}
				disconnect_client(server, fd);
				continue;
			}

			if (poll_fd.revents & POLLHUP)
			{
				if (poll_fd.fd != server.listen_fd)
					disconnect_client(server, fd);
				else
					i++;
				continue;
			}

			if (poll_fd.revents & POLLIN)
			{
				if (poll_fd.fd == server.listen_fd)
					accept_new_client(server);
				else if (read_from_client(server, fd))
					continue;
			}
			i++;
		}
	}
}
