#include <csignal>
#include <iostream>
#include <exception>
#include <limits> // std::numeric_limits
#include "../lib/server.hpp"
#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

static bool	input_validation(int argc, char **argv, uint16_t *port, size_t *pass_len);
static void	initialize_signal_handler(struct sigaction &sa);
static void	initialize_server(t_IRC_Server &server, uint16_t port,
	            const char *input_password, size_t pass_len);

volatile sig_atomic_t requested_shutdown = 0;

void signal_handler(int sig)
{
	(void) sig;
	requested_shutdown = 1;
}

int main(int argc, char **argv)
{
	uint16_t	port = 0;
	size_t		pass_len = 0;

	if (!input_validation(argc, argv, &port, &pass_len))
		return 2;

	struct sigaction sa = {};
	initialize_signal_handler(sa);

	try {
		/* 'server' contains different containers whose allocators may throw */
		t_IRC_Server	server = {};
		initialize_server(server, port, argv[2], pass_len);

		/* This nested try-catch grants the possibility to monitor the server's
		 * bitmask once its loop is over, before 'server' goes out of scope */
		try {
			create_listener(server);
			if (!is_flag_set(server.state, server.FATAL_ERROR)
					&& !requested_shutdown)
				server_loop(server);

		} catch (const std::exception &e) {
			std::cerr << "Exception caught: " << e.what() << std::endl;
			server.state |= t_IRC_Server::FATAL_ERROR;
		}

		/* the FATAL_ERROR server flag may be set:
		*    • at the catch block just above
		*    • during runtime, after system call failures
		* Both paths would converge here, causing the boolean to be true */
		if (is_flag_set(server.state, t_IRC_Server::FATAL_ERROR))
			return 1;

	} catch (const std::exception &e) {
		/* catches any allocation/initialization failure of 'server' */
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

static bool	input_validation(int argc, char **argv, uint16_t *port,
	            size_t *pass_len)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 0;
	}

	// convert port argument to 16 bit unsigned, refuse inappropriate input
	if (!parse_positive_integer_and_validate_input(argv[1], *port))
	{
		std::cerr
			<< "Port \"" << argv[1] << "\" is invalid. Accepted range: 1 to "
			<< std::numeric_limits<uint16_t>::max() << ", inclusive. "
			"Only numeric characters are accepted."
			<< std::endl;
		return 0;
	}

	*pass_len = validate_password_and_strlen(argv[2]);
	if (!*pass_len)
	{
		std::cerr
			<< "Password '" << argv[2] << "' is either empty or contains "
			"invalid non-printable characters." << std::endl;
		return 0;
	}
	return 1;
}

static void	initialize_signal_handler(struct sigaction &sa)
{
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

static void	initialize_server(t_IRC_Server &server, uint16_t port,
	            const char *input_password, size_t pass_len)
{
	server.port = port;
	server.password = std::string_view{input_password, pass_len};
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
