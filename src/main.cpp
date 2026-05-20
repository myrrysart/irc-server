#include <iostream>
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    std::cout << "ircserv starting on port " << argv[1] << std::endl;

    t_IRC_Server server = {};
    server.port = atoi(argv[1]);
    create_listener(server);
    server_loop(server);
    return 0;
}

/*
 * steps to establish connection:
 * 1. Create socket
 * 2. Bind socket to port
 * 3. start listening for connections
 * 4. poll for ready fd's
 * 5. Accept client and add (POLLIN flag) to poll set and client map
 */
