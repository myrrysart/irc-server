#include "../lib/server.hpp"
#include <sys/socket.h>

void	fatal_server_error(const char* msg, int fd)
{
	std::perror(msg);
	if (fd>=0)
		close(fd);
	exit(1);
}

int	shutdown_server(t_IRC_Server *server)
{
	server->state = 0;
	if (server->listen_fd >= 0)
		close(server->listen_fd);

	for (auto &[fd, client] : server->clients)
		close(fd);

	server->clients.clear();
	server->poll_fds.clear();
	return 0;
}
