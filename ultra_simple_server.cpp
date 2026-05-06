#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

int setup_socket(int port)
{
	/*
	 * Ask the OS for a new TCP socket
	 * AF_INET = IPv4 address family
	 * SOCK_STREAM = TCP protocol,
	 * protocol = 0 (default protocol)
	*/
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::perror("socket");
		exit(1);
	}

	// enable quick port reuse. Just to avoid the "Address already in use" error. Not sure if needed in final version.
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::perror("setsockopt");
		close(server_fd);
		exit(1);
	}

	// Configure the server address and port.
	sockaddr_in socket_addr{};
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY); //  = any available interface
	socket_addr.sin_port = htons(port);

	/*
	 * Bind the socket to the configured address.
	 * Sort of polymorphism. bind() uses generic sockaddr. Cast sockaddr_in to match the function signature.
	*/
	if (bind(server_fd, reinterpret_cast<sockaddr*>(&socket_addr), sizeof(socket_addr)) < 0)
	{
		std::perror("bind");
		close(server_fd);
		exit(1);
	}

	// Start listening for incoming connections.
	if (listen(server_fd, 10) < 0) // 10 max num of pending connections.
	{
		std::perror("listen");
		close(server_fd);
		exit(1);
	}

	std::cout << "Listening on port " << port << "\n";
	return server_fd;
}

/*
 * the second parameters is a pointer to a struct where accept() writes the clients address.
 * The format depends on the protocol used. In out case sockaddr_in (ip4v). So sort of polymorphism again?
 */

int	accept_connetion(int server_fd)
{
	int client_fd = accept(server_fd, nullptr, nullptr);
	if (client_fd < 0)
	{
		std::perror("accept");
		close(server_fd);
		exit(1);
	}
	return client_fd;
}

void echo_loop(int client_fd)
{
	std::array<char, 512> echo_buf;
	while(1)
	{
		ssize_t received = recv(client_fd, echo_buf.data(), echo_buf.size(), 0);

		if (received == 0)
		{
			std::cout << "Client disconnected." << std::endl;
			break;
		}
		if (received < 0)
		{
			std::perror("recv");
			break;
		}
		if (received > 0)
		{
			std::string message(echo_buf.data(), received);
			std::cout << "Received: " << message << std::endl;

			message = "Echo: " + message;
			send(client_fd, message.data(), message.size(), 0);
		}
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
		return 1;
	}
	int server_fd = setup_socket(atoi(argv[1]));
	std::cout << "To test type ´nc localhost " << argv[1] << "' in a different terminal. "
		<< "Then write something to see it echoed back." << std::endl;
	int client_fd = accept_connetion(server_fd);
	echo_loop(client_fd);
	close(client_fd);
	close(server_fd);
	std::cout << "Server done." << std::endl;
	return 0;
}
