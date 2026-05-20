#include <iostream>
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
