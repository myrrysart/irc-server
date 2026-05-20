#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"

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
