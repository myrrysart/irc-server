#include <iostream>
#include <string>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char	buf[t_parser::buf_size];

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received <= 0)
		return true;

	if (server.clients[fd].state & t_IRC_Client::Flags::DISCARD_MSG)
		handle_message_to_discard(server.clients[fd], buf, received);
	else
		server.clients[fd].received_message_buffer.append(buf, received);

	return false;
}

void 	queue_out_message(t_IRC_Server &server,int fd, std::string msg)
{
	server.clients[fd].send_message_buffer += msg;
	server.clients[fd].send_message_buffer += "\n";
}

void	handle_client_message(t_IRC_Server &server, int fd)
{
	std::string		&buf = server.clients[fd].received_message_buffer;
	t_IRC_Client	&client = server.clients[fd];
	size_t			pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		prepare_and_parse_message(pos, buf, client);
		queue_out_message(server, fd, buf.substr(0, pos));
		flush_client(server, fd);
		buf.erase(0, pos + 1);
	}
	check_for_too_long_message(buf, client);
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd) {return pfd.fd == fd; });
	std::cout << "Client disconnected.\n";
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}
