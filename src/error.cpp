#include "../lib/server.hpp"
#include <sys/socket.h>

#include <iostream> // for std::cerr and its insertion operator

void	fatal_server_error(const char* msg, int fd)
{
	std::perror(msg);
	if (fd>=0)
		close(fd);
	exit(1);
}

int	shutdown_server(t_IRC_Server *server)
{
	if (server->listen_fd >= 0)
		close(server->listen_fd);

	for (auto &[fd, client] : server->clients)
		close(fd);

	server->clients.clear();
	server->poll_fds.clear();
	return 0;
}

/* When calling this function:
* • 'filename': pass the macro '__FILE__'
* • 'line_number': pass the macro '__LINE__' */
void	log_error(const char *error, const char *filename, int line_number,
            bool is_exception)
{
	std::cerr << "ERROR: ";
	if (is_exception)
		std::cerr << "Exception caught: ";
	std::cerr << error <<
		" (file: " << filename << ", line: " << line_number << ')' << std::endl;
}
