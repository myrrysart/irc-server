#include <iostream>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

void	create_listener(t_IRC_Server &server)
{
	server.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.listen_fd < 0)
		fatal_server_error("socket", -1);

	int	opt = 1;
	if (setsockopt(server.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		fatal_server_error("setsockopt", server.listen_fd);

	sockaddr_in	socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_addr.sin_port = htons(server.port);

	if (bind(server.listen_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
		fatal_server_error("bind", server.listen_fd);

	if (listen(server.listen_fd, MAX_PENDING_CONNECTIONS) < 0)
		fatal_server_error("listen", server.listen_fd);

	std::cout << "Listening on port " << server.port << "\n";
}
