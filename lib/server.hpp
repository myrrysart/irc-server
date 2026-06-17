#ifndef SERVER_HPP
# define SERVER_HPP

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "irc_fatstruct.hpp"

extern volatile sig_atomic_t requested_shutdown;

void	fatal_server_error(const char* msg, int fd);
int		shutdown_server(t_IRC_Server *server);

void	create_listener(t_IRC_Server &server);

void	server_loop(t_IRC_Server &server);
void	accept_new_client(t_IRC_Server &server);

bool	recv_from_client(t_IRC_Server &server, int fd);
void	handle_client_message(t_IRC_Client &client, t_IRC_Server &server);
void	disconnect_client(t_IRC_Server &server, int fd);

void	send_messages_to_all_clients(t_IRC_Server &server);

#endif
