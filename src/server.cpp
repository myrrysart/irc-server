#include <iostream>
#include <cstring> // for std::memmove() and std::strerror()
#include <string>
#include <cerrno>
#include <exception>
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/channel.hpp"

static void	initialize_hostname(const struct in_addr *addr, char *hostname)
{
	if (!inet_ntop(AF_INET, addr, hostname, INET_ADDRSTRLEN))
	{
		if (errno == EAFNOSUPPORT)
			log_error("Failure to initialize hostname; "
			"client's address is neither AF_INET nor AF_IFNET6. "
			"Setting as 'unknown'.",
			"inet_ntop", __FILE__, __LINE__);
		else
			log_error("Failure to initialize hostname; "
			"Size provided for string conversion is insufficient. "
			"Setting as 'unkonwn'.",
			"inet_ntop", __FILE__, __LINE__);

		// sizeof() includes a '\0', making the array safe for appending
		(void)std::memmove(hostname, "unknown", sizeof("unknown"));
	}
}

static bool	setup_client(t_IRC_Server &server, int client_fd, struct sockaddr_in const &client_addr)
{
	int	one = 1;
	if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) < 0)
	{
		log_error(std::strerror(errno), "setsockopt", __FILE__, __LINE__);
		close(client_fd);
		return false;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		log_error(std::strerror(errno), "fcntl", __FILE__, __LINE__);
		close(client_fd);
		return false;
	}

	// WARN: both of the next calls may heap allocate via std::vector and std::unordered_map.
	// These can throw exceptions, which previously would not be caught by our program.
	// If either of those fails, the client_fd has to be closed here, as it might
	// not have been integrated to the poll_fds vector.
	try {
		server.poll_fds.push_back(pollfd{client_fd, POLLIN, 0});
		server.clients[client_fd] = t_IRC_Client{};
	} catch (const std::exception &e) {
		close(client_fd);
		throw; // throws the same exception that was just caught
	}

	t_IRC_Client	&client = server.clients[client_fd];
	client.fd = client_fd;
	client.addr = client_addr;

	/* Initialize default nickname to '*'. This is an invalid nickname, but it
	* is crucial for sending numeric replies from the server to the client when
	* registration is still ongoing, without leading to parsing errors on the
	* client side. */
	client.nick_buf[0] = '*';
	client.nick = std::string_view{client.nick_buf, 1};
	initialize_hostname(&client.addr.sin_addr, client.hostname);

	return true;
}

static void	notify_client_that_server_is_full(int fd, const char *server_name,
				const struct in_addr *addr)
{
	std::string	error_msg;
	char		hostname[INET_ADDRSTRLEN] = {};

	initialize_hostname(addr, hostname);
	append_common_error_prefix(error_msg, server_name, hostname);
	error_msg += " (Server is full)\r\n";
	(void)send(fd, error_msg.c_str(), error_msg.size(), MSG_NOSIGNAL);
}

void	accept_new_client(t_IRC_Server &server)
{
	sockaddr_in client_addr{};
	socklen_t	client_addr_len = sizeof(client_addr);
	int			client_fd = accept(server.listen_fd, (sockaddr*)&client_addr, &client_addr_len);
	if (client_fd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		log_error(std::strerror(errno), "accept", __FILE__, __LINE__);
		return;
	}
	if (server.clients.size() >= MAX_CLIENTS)
	{
		notify_client_that_server_is_full(client_fd, server.name, &client_addr.sin_addr);
		log_error("Server is full", "server", __FILE__, __LINE__);
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
		{
			// WARN: What error log should be communicated in this case? Next line just a draft.
			// log_error("Fatal failure", "listening socket", __FILE__, __LINE__);
			server.state |= server.FATAL_ERROR;
			requested_shutdown = 1;
			return true;
		}
		broadcast_non_requested_disconnect_msg(server.clients[fd]);
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
		// NOTE: The following check is in case the client should be disconnected,
		// there might still be messages to be sent to it before disconnecting.
		// This will be handled in the subsequent send loop. But here, the server
		// can simply ignore any input from that client, it is irrelevant.
		if (is_flag_set(server.clients[fd].state, t_IRC_Client::DISCONNECT))
			return false;
		if (recv_from_client(server, fd))
		{
			broadcast_non_requested_disconnect_msg(server.clients[fd]);
			disconnect_client(server, fd);
			return true;
		}
		if (!requested_shutdown)
			handle_client_message(server.clients[fd], server);
	}
	return false;
}

void	server_loop(t_IRC_Server &server)
{
	server.poll_fds.push_back(pollfd{server.listen_fd, POLLIN, 0});
	std::cout << "server running at port " << server.port << std::endl;

	while (!requested_shutdown)
	{
		if (poll(server.poll_fds.data(), static_cast<nfds_t>(server.poll_fds.size()),
			t_IRC_Server::poll_timeout) < 0)
		{
			if (errno == EINTR)
				continue;
			else
			{
				log_error(strerror(errno), "poll", __FILE__, __LINE__);
				server.state |= server.FATAL_ERROR;
				requested_shutdown = 1;
				return ;
			}
		}

		// pollin event loop
		for (size_t i = 0; i < server.poll_fds.size(); )
		{
			if (requested_shutdown)
				return;
			int		fd = server.poll_fds[i].fd;
			short	rev = server.poll_fds[i].revents;
			bool	is_removed = handle_poll_event(server, fd, rev);
			if (!is_removed)
				i++;
		}

		// send messages loop
		send_messages_to_all_clients(server);
	}
}
