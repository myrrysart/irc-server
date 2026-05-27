#include <iostream>
#include <string_view>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static constexpr std::size_t	buf_size = 512;
	char		buf[buf_size];
	std::string	&msg = server.clients[fd].received_message_buffer;

	ssize_t	received = recv(fd, buf, buf_size, 0);
	if (received <= 0)
		return true;

	// NOTE: Handle message with over 512 bytes:
	// - Truncate the last 2 bytes: at index 510, insert "\r\n"
	// or - send ERR_INPUTTOOLONG (417).
	// This is based on "Compatibility with incorrect software" section on
	// modern.ircdocs.horse/#source
	// if (received >= buf_size)
	// {
	//
	//
	// }
		// Alternative 'truncation' version:
		// buf[buf_size - 2] = '\r';
		// buf[buf_size - 1] = '\n';
	// }
	// FIXME: Messages over 512 requires discarding the rest of the message,
	// which would only get received later on!

	// FIXME: Have to add a check for the buffer being over 512 as well!
	// FIXME: just work in progress right now!
	if (msg.length() > buf_size || (msg.length() == buf_size && msg[buf_size - 1] != '\n'))
	{
		// TODO: handle the error:
			// - communicate: ERR_INPUTTOOLONG (417)
			// - set some flag so that you can DISCARD rest of incoming message
			// - add a check for that DISCARD flag at the top of this function?
			// - erase the msg buffer?

		return true;
	}
	// FIXME:
	// Also handle case where there is only the '\n', not both "\r\n"

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
