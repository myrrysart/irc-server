#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <array>

#include "../lib/irc_fatstruct.hpp"

int setup_socket(int port)
{
	/*
	 * Ask the OS for a new TCP socket
	 * AF_INET = IPv4 address family
	 * SOCK_STREAM = TCP protocol,
	 * protocol = 0 (default protocol)
	*/
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::perror("socket");
		exit(1);
	}

	// enable quick port reuse. Just to avoid the "Address already in use" error. Not sure if needed in final version.
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::perror("setsockopt");
		close(server_fd);
		exit(1);
	}

	// Configure the server address and port.
	sockaddr_in socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY); //  = any available interface
	socket_addr.sin_port = htons(port);

	/*
	 * Bind the socket to the configured address.
	 * Sort of polymorphism. bind() uses generic sockaddr. Cast sockaddr_in to match the function signature.
	*/
	if (bind(server_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
	{
		std::perror("bind");
		close(server_fd);
		exit(1);
	}

	// Start listening for incoming connections.
	if (listen(server_fd, 10) < 0) // 10 max num of pending connections.
	{
		std::perror("listen");
		close(server_fd);
		exit(1);
	}

	std::cout << "Listening on port " << port << "\n";
	return server_fd;
}

/*
 * the second parameters is a pointer to a struct where accept() writes the clients address.
 * The format depends on the protocol used. In out case sockaddr_in (ip4v). So sort of polymorphism again?
 */

void disconnect_client(std::vector<pollfd>& fds, size_t index)
{
	close(fds[index].fd);
	fds.erase(fds.begin() + static_cast<std::ptrdiff_t>(index));
}

void accept_new_client(std::vector<pollfd>& fds, int server_fd)
{
	int client_fd = accept(server_fd, nullptr, nullptr);
	if (client_fd < 0)
	{
		std::perror("accept");
		return;
	}
	fds.push_back(pollfd{client_fd, POLLIN, 0});
}

bool read_from_client(std::vector<pollfd>& fds, size_t index, std::array<char, 512>& echo_buf)
{
	int fd = fds[index].fd;
	ssize_t received = recv(fd, echo_buf.data(), echo_buf.size(), 0);
	if (received == 0)
	{
		std::cout << "Client disconnected.\n";
		disconnect_client(fds, index);
		return true;
	}
	if (received < 0)
	{
		std::perror("recv");
		disconnect_client(fds, index);
		return true;
	}
	if (received > 0)
	{
		std::string message(echo_buf.data(), static_cast<size_t>(received));
		std::cout << "Received: " << message << std::endl;
		message = "Echo: " + message;
		send(fd, message.data(), message.size(), 0);
	}
	return false;
}

void server_loop(int server_fd)
{
	std::array<char, 512> echo_buf;

	.push_back(pollfd{server_fd, POLLIN, 0});

	while (1)
	{
		if (poll(fds.data(), static_cast<nfds_t>(fds.size()), -1) < 0)
		{
			std::perror("poll");
			break;
		}

		size_t i = 0;
		while (i < fds.size())
		{
			pollfd& pfd = fds[i];

			if (pfd.revents & POLLERR)
			{
				if (pfd.fd == server_fd)
				{
					std::cerr << "Listen socket error.\n";
					return;
				}
				disconnect_client(fds, i);
				continue;
			}

			if (pfd.revents & POLLHUP)
			{
				if (pfd.fd != server_fd)
					disconnect_client(fds, i);
				else
					i++;
				continue;
			}

			if (pfd.revents & POLLIN)
			{
				if (pfd.fd == server_fd)
					accept_new_client(fds, server_fd);
				else if (read_from_client(fds, i, echo_buf))
					continue;
			}

			i++;
		}
	}
}


/*
 * 4. Refactor `ultra_simple_server.cpp`
 - `server_loop(t_IRC_Server& server)` — owns `while(1) { poll(); poll_loop(); }`, no local buffers/vectors
 - `poll_loop(t_IRC_Server& server)` — iterates `server.poll_fds`, dispatches to handlers
 - `accept_new_client(t_IRC_Server& server)` — pulls from client pool, pushes to poll_fds
 - `read_from_client(t_IRC_Server& server, size_t i)` — uses `server.clients[...].received_message_buffer` instead of shared `echo_buf`
 - `disconnect_client(t_IRC_Server& server, size_t i)` — releases client slot, erases from poll_fds
 - Remove the now-unnecessary `echo_buf` param
 */
