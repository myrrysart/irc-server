#include <iostream>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    std::cout << "ircserv starting on port " << argv[1] << std::endl;
    return 0;
}
