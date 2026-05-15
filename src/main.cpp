#include <iostream>
#include "../lib/irc_fatstruct.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    std::cout << "ircserv starting on port " << argv[1] << std::endl;

    // init irc_struct
    t_IRC_Server server = {};
    server.port = atoi(argv[1]);
    setup_socket(server);
    server_loop(server);
    return 0;
}
