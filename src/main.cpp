#include <csignal>
#include <iostream>
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

volatile sig_atomic_t requested_shutdown = 0;

void signal_handler(int sig)
{
	(void) sig;
	requested_shutdown = 1;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	struct sigaction sa = {};
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	t_IRC_Server server = {};
	server.port = atoi(argv[1]);

	if (init_password(argv[2], server.password) == -1)
	{
		std::cerr
			<< "Password '" << argv[2] << "' is either empty or contains "
			"invalid non-printable characters." << std::endl;
		return 1;
	}

	create_listener(server);
	server_loop(server);
	shutdown_server(&server);
	return 0;
}

/*
 * steps to establish connection:
 * create listener:
 * 	1. Create socket
 * 	2. Bind socket to port
 * 	3. start listening for connections
 * server loop:
 * 	1. poll for ready fd's
 * 	2. Accept client and add (POLLIN flag) to poll set and client map
 */
