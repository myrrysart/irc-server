#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

void	create_listener(t_IRC_Server &server)
{
	server.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.listen_fd < 0)
	{
		set_fatal_error_flag_and_log(server.state, "socket", __FILE__, __LINE__);
		return;
	}

	if (fcntl(server.listen_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		set_fatal_error_flag_and_log(server.state, "fcntl", __FILE__, __LINE__);
		return;
	}

	int	opt = 1;
	if (setsockopt(server.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		set_fatal_error_flag_and_log(server.state, "setsockopt", __FILE__, __LINE__);
		return;
	}

	sockaddr_in	socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_addr.sin_port = htons(server.port);

	if (bind(server.listen_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
	{
		set_fatal_error_flag_and_log(server.state, "bind", __FILE__, __LINE__);
		return;
	}

	if (listen(server.listen_fd, MAX_PENDING_CONNECTIONS) < 0)
		set_fatal_error_flag_and_log(server.state, "listen", __FILE__, __LINE__);
}
