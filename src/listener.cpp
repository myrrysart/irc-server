#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

#include <cstring> // for std::strerror()
#include <cerrno>

void	create_listener(t_IRC_Server &server)
{
	server.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.listen_fd < 0)
	{
		log_error(std::strerror(errno), "socket", __FILE__, __LINE__);
		server.state |= server.FATAL_ERROR;
		return;
	}

	if (fcntl(server.listen_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		log_error(std::strerror(errno), "fcntl", __FILE__, __LINE__);
		server.state |= server.FATAL_ERROR;
		return;
	}

	int	opt = 1;
	if (setsockopt(server.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		log_error(std::strerror(errno), "setsockopt", __FILE__, __LINE__);
		server.state |= server.FATAL_ERROR;
		return;
	}

	sockaddr_in	socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_addr.sin_port = htons(server.port);

	if (bind(server.listen_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
	{
		log_error(std::strerror(errno), "bind", __FILE__, __LINE__);
		server.state |= server.FATAL_ERROR;
		return;
	}

	if (listen(server.listen_fd, MAX_PENDING_CONNECTIONS) < 0)
	{
		log_error(std::strerror(errno), "listen", __FILE__, __LINE__);
		server.state |= server.FATAL_ERROR;
	}
}
