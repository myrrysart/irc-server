#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <array>

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

void disconnect_client(t_IRC_Server &server, size_t index)
{
	close(server.fds[index].fd);
	server.fds.erase(server.fds.begin() + static_cast<std::ptrdiff_t>(index));
}

void accept_new_client(t_IRC_Server &server)
{
	int client_fd = accept(server.listen_fd, nullptr, nullptr);
	if (client_fd < 0)
	{
		std::perror("accept");
		return;
	}
	server.fds.push_back(pollfd{client_fd, POLLIN, 0});
}

bool read_from_client(t_IRC_Server &server, int index)
{
	int fd = server.fds[index].fd;
	ssize_t received = recv(fd, server.clients[index].received_message_buffer.data(),
			server.clients[index].received_message_buffer.size(), 0);
	if (received == 0)
	{
		std::cout << "Client disconnected.\n";
		disconnect_client(server, index);
		return true;
	}
	if (received < 0)
	{
		std::perror("recv");
		disconnect_client(server, index);
		return true;
	}
	if (received > 0)
	{
		std::cout << "Received: " << server.clients[index].received_message_buffer.data() << std::endl;
		send(fd, server.clients[index].received_message_buffer.data(), server.clients[index].received_message_buffer.size(), 0);
	}
	return false;
}

void server_loop(t_IRC_Server &server)
{
	server.fds.push_back(pollfd{server.listen_fd, POLLIN, 0});
	while (1)
	{
		if (poll(server.fds.data(), static_cast<nfds_t>(server.fds.size()), -1) < 0)
		{
			std::perror("poll");
			break;
		}

		size_t i = 0;
		while (i < server.fds.size())
		{
			pollfd& poll_fd = server.fds[i];

			if (poll_fd.revents & POLLERR)
			{
				if (poll_fd.fd == server.listen_fd)
				{
					std::cerr << "Listen socket error.\n";
					return;
				}
				disconnect_client(server, i);
				continue;
			}

			if (poll_fd.revents & POLLHUP)
			{
				if (poll_fd.fd != server.listen_fd)
					disconnect_client(server, i);
				else
					i++;
				continue;
			}

			if (poll_fd.revents & POLLIN)
			{
				if (poll_fd.fd == server.listen_fd)
					accept_new_client(server);
				else if (read_from_client(server, i ))
					continue;
			}
			i++;
		}
	}
}
/*
 * fds` index ≠ `clients` index**
    In `read_from_client` you use `server.clients[index]`
    with the same `index` as **`server.fds[i]`**. But **`fds[0]`
    is the listen socket**, not a client. The first real client
    is **`fds[1]`**, while the first free client slot is probably
    **`clients[0]`** (or whatever slot you allocate).
    * So today the indices only match if you accidentally treat
    * slot 0 as listen, which you do not — you will hit the
    * ** *wrong `t_IRC_Client`** or garbage `fd` in the client struct.

 2. **`accept_new_client` only pushes `pollfd`**
    It never picks a **`clients[]` slot**,
    never sets **`clients[k].fd`**,
    never ties **`fds` entry ↔ client slot**
    (nor the `poll_index` idea you discussed).
    Until that exists, the fat struct is not really
    “in use” for clients.

 3. **`recv` into `std::string`**
    With C++17, `data()` is writable, but **`size()` is the current string length**, not “room to read into.” For an empty buffer that is often **0-length `recv`**. You usually **`resize`/`reserve`** a read window or use a small `std::array<char, N>` and append.

 4. **Build**
    Your **`Makefile` only compiles `src/main.cpp`**. `setup_socket` / `server_loop` live in **`server.cpp`**, so a normal `make` will **not link** unless you add `src/server.cpp` (or merge into one `.cpp`). So you may not even be running this code via the Makefile yet.

 5. **Small**
    `argv[2]` password is never stored in **`server.password`**.
 */
