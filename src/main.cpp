#include <csignal>
#include <iostream>
#include <exception>
#include <limits> // std::numeric_limits
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

	// convert port argument to 16 bit unsigned, refuse inappropriate input.
	uint16_t	port = 0;
	if (!parse_positive_integer_and_validate_input(argv[1], port))
	{
		std::cerr
			<< "Port \"" << argv[1] << "\" is invalid. Accepted range: 1 to "
			<< std::numeric_limits<uint16_t>::max() << ", inclusive. "
			"Only numeric characters are accepted."
			<< std::endl;
		return 1;
	}

	size_t	password_len = validate_password_and_strlen(argv[2]);
	if (password_len <= 0)
	{
		std::cerr
			<< "Password '" << argv[2] << "' is either empty or contains "
			"invalid non-printable characters." << std::endl;
		return 1;
	}

	struct sigaction sa = {};
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	t_IRC_Server	server = {};

	server.port = port;
	server.password = std::string_view{argv[2], password_len};

	try {
		create_listener(server);
		if (!is_flag_set(server.state, server.FATAL_ERROR) && !requested_shutdown)
			server_loop(server);

	} catch (const std::exception &e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		server.state |= t_IRC_Server::FATAL_ERROR;
	}
	// FATAL_ERROR flag may be set by a caught exception or by syscall failures
	if (is_flag_set(server.state, server.FATAL_ERROR))
		return 1;
	return 0;
}

// WARN: should sending SIGINT to the program return 0 (which is the case currently)?

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
