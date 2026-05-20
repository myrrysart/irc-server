#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <string>
#include <sys/poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "irc_fatstruct.hpp"

void	fatal_server_error(const char* msg, int fd);

void	create_listener(t_IRC_Server &server);

void	server_loop(t_IRC_Server &server);

void	accept_new_client(t_IRC_Server &server);
bool	recv_from_client(t_IRC_Server &server, int fd);
void	handle_client_message(t_IRC_Client &client);
void	disconnect_client(t_IRC_Server &server, int fd);
