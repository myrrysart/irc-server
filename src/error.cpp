#include "../lib/irc_fatstruct.hpp"
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <iostream> // for std::cerr and its insertion operator
#include <cstring>  // for std::strerror()
#include <string>
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

void	append_error_msg_quit(t_IRC_Client &quitter, const char *server_name)
{
	std::string	&output_buf = quitter.send_message_buffer;

	output_buf += ':';
	output_buf += server_name;
	output_buf += " ERROR: Closing connection: ";
	output_buf += quitter.nick;
	output_buf += '[';
	output_buf += quitter.hostname;
	output_buf += "] (Quit: ";

	// Append client's reason of departure, if provided.
	// Handling for no reason / empty reason mimics the one required by the
	// protocol for fellow channelers alert message, i.e.: "(Quit: )"
	if (quitter.parser.n_params)
		output_buf += quitter.parser.params[0];

	output_buf += ")\r\n";
}
