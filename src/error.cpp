#include "../lib/irc_fatstruct.hpp"
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <iostream> // for std::cerr and its insertion operator
#include <cstring>  // for std::strerror()
#include <cerrno>

s_IRC_Server::~s_IRC_Server()
{
	if (this->listen_fd >= 0)
		close(this->listen_fd);

	for (auto &[fd, client] : this->clients)
		close(fd);

	// NOTE: No need to manually clear the std::vector and std::unordered_map
	// containers: their own destructors will get called automatically.
}

/* When calling this function:
* • 'error': Description of the failure. Where applicable, can just be std::strerror(errno)
* • 'context': Name of function that failed (for example: "send", "fcntl")
* • 'filename': pass the macro '__FILE__'
* • 'line_number': pass the macro '__LINE__' */
void	log_error(const char *error, const char *context, const char *filename,
            const int line_num)
{
	std::cerr
		<< "ERROR. " << context << ": " << error
		<< " (file: " << filename << ", line: " << line_num << ')'
		<< std::endl;
}

void	set_fatal_error_flag_and_log(t_bmask &state, const char *context,
            const char *filename, const int line_num)
{
	state |= t_IRC_Server::FATAL_ERROR;
	log_error(std::strerror(errno), context, filename, line_num);
}
