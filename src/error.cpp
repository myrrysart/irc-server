#include "../lib/server.hpp"

void	fatal_server_error(const char* msg, int fd)
{
	std::perror(msg);
	if (fd>=0)
		close(fd);
	exit(1);
}
