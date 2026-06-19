#include "../lib/server.hpp"
#include <sys/socket.h>

#include <iostream> // for std::cerr and its insertion operator

void	shutdown_server(t_IRC_Server *server)
{
	if (!server)
		return;

	if (server->listen_fd >= 0)
		close(server->listen_fd);

	for (auto &[fd, client] : server->clients)
		close(fd);

	server->clients.clear();
	server->poll_fds.clear();
}

/* When calling this function:
* • 'error': Description of the failure. Where applicable, can just be std::strerror(errno)
* • 'context': Name of function that failed (for example: "send", "fcntl")
* • 'filename': pass the macro '__FILE__'
* • 'line_number': pass the macro '__LINE__' */
void	log_error(const char *error, const char *context, const char *filename,
            int line_num)
{
	std::cerr
		<< "ERROR. " << context << ": " << error
		<< " (file: " << filename << ", line: " << line_num << ')'
		<< std::endl;
}

void	log_exception(const char *what)
{
	std::cerr << "Exception caught: " << what << std::endl;
}
