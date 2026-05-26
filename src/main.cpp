#include <iostream>
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	t_IRC_Server server = {};
	server.port = atoi(argv[1]);
	create_listener(server);
	server_loop(server);
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
