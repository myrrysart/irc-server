#include <iostream>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	char	buf[512];

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received <= 0)
		return true;
	// FIXME: add a truncation of the last 2 bytes: at index 510, insert "\r\n" - 
	// or send ERR_INPUTTOOLONG (417).
	// This is based upon "Compatibility with incorrect software" section on
	// modern.ircdocs.horse/#source
	if (received >= 512)
	{
		buf[510] = '\r';
		buf[511] = '\n';
	}
	server.clients[fd].received_message_buffer.append(buf, received);
	return false;
}

void	handle_client_message(t_IRC_Client &client)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos;

	while((pos = buf.find('\n')) != std::string::npos)
	{
		bool	has_carriage_return = 0;
		if (pos >= 1 && buf[pos - 1] == '\r')
			has_carriage_return = 1;

		std::string_view msg{&buf[0], pos - has_carriage_return};

		std::cout	<< "Received from " << client.fd << " : " << msg << std::endl;

		//
		// TODO: parsing happens here.
		// Careful, 'msg' will not survive the end of this iteration of while loop!





		//
		buf.erase(0, pos + 1 + has_carriage_return);
	}
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
	std::cout << "Client disconnected.\n";
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}
