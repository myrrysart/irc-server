#include <iostream>
#include <string_view>

#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char		buf[t_parser::buf_size];
	std::string	&msg = server.clients[fd].received_message_buffer;

	ssize_t	received = recv(fd, buf, t_parser::buf_size, 0);
	if (received <= 0)
		return true;


	if (server.clients[fd].state & t_IRC_Client::Flags::DISCARD_MSG)
	{
		// received_message_buffer should be cleaned up by now.
		// look for the new line in 'buf'
		// if there is a new line:
		// 		- move the trailing characters (from 1 after the
		// 		  newline character until the last 'received') to the beginning
		// 		  of buf OR append already only those trailing characters to
		// 		  msg - but then the following append() call should be in an else statement.
		// 		- update client's state: unset DISCARD_MSG flag
		// 		- let the function continue
		// if there is no new line:
		// 		- ignore what 'buf' holds.
		// 		- do not change client state.
		// 		- return false;

		std::string	temp_buf{buf};
		size_t	pos = temp_buf.find('\n'); // WARN: try without temp_buf, but with buf instead? But would probably not compile!
		// if (pos == std::string::npos) // this if statement is probably redundant.
		// 	return false;

		if (pos != std::string::npos)
		{
			// newline has been found in the buffer.
			// append the trailing part after the new line
			if (pos < received - 1) // otherwise, there is nothing to append.
				server.clients[fd].received_message_buffer.append(buf[pos + 1], received - pos - 1);
			// unset DISCARD_MSG flag  WARN: is this correct?
			server.clients[fd].state &= ~t_IRC_Client::Flags::DISCARD_MSG;

		}
	}
	else
		server.clients[fd].received_message_buffer.append(buf, received);

	// NOTE: Handle message with over 512 bytes:
	// - Truncate the last 2 bytes: at index 510, insert "\r\n"
	// or - send ERR_INPUTTOOLONG (417).
	// This is based on "Compatibility with incorrect software" section on
	// modern.ircdocs.horse/#source
	// if (received >= t_parser::buf_size)
	// {
	//
	//
		// Alternative 'truncation' version:
		// buf[t_parser::buf_size - 2] = '\r';
		// buf[t_parser::buf_size - 1] = '\n';
	// }
	// FIXME: Messages over 512 requires discarding the rest of the message,
	// which would only get received later on!

	// FIXME: the next block is rather wrong: you might have appended multiple
	// valid messages from one go of recv(), and this will discard all of them !!!!!

	// FIXME: Have to add a check for the buffer being over 512 as well!
	// FIXME: just work in progress right now!

	// FIXME: make sure that the edge case where huge amounts of bytes WITHOUT ANY
	// NEWLINE character in them would not simply go unnoticed!
	/*
	if (msg.length() > t_parser::buf_size
		|| (msg.length() == t_parser::buf_size && msg[t_parser::buf_size - 1] != '\n'))
	{
		// TODO: handle the error:
			// - communicate: ERR_INPUTTOOLONG (417)
			// - set some flag so that you can DISCARD rest of incoming message
			// - add a check for that DISCARD flag at the top of this function?
			// - erase the msg buffer?

		// - set some flag so that you can DISCARD rest of incoming message
		server.clients[fd].state |= t_IRC_Client::Flags::DISCARD_MSG;

		return true;
	}
	*/

	return false;
}

void	handle_client_message(t_IRC_Client &client)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		bool	has_carriage_return = 0;

		if (pos >= 1 && buf[pos - 1] == '\r')
			has_carriage_return = 1;

		if (pos >= t_parser::buf_size)
		{
		// TODO: handle the error:
			// - communicate: ERR_INPUTTOOLONG (417)
			// - set some flag so that you can DISCARD rest of incoming message
			// - add a check for that DISCARD flag at the top of recv_from_client()
			// - erase the msg buffer?

			// - set some flag so that you can DISCARD rest of incoming message
			client.state |= t_IRC_Client::Flags::DISCARD_MSG;
			// - erase the msg buffer
			buf.erase(0, pos + 1);
		}


		// candidate message detected: prepare string_view for parsing.
		std::string_view msg{&buf[0], pos - has_carriage_return};

		std::cout	<< "Received from " << client.fd << " : " << msg << std::endl;

		// TODO: parsing happens here.
		// Careful, 'msg' will not survive the end of this iteration of while loop!





		//
		// buf.erase(0, pos + 1 + has_carriage_return); // seems wrong! pos is on '\n'
		buf.erase(0, pos + 1);
	}

	// when we arrive here:
	// - 'buf' holds only the trailing, non processed bytes. It does not contain
	// a newline. But it can still hold a message larger than 512 bytes! And its
	// message might not be ready!
	if (buf.length() >= t_parser::buf_size)
	{
		// TODO:
		// - communicate: ERR_INPUTTOOLONG (417)
		client.state |= t_IRC_Client::Flags::DISCARD_MSG;
		buf.clear(); // deletes the whole buffer

		// FIXME: If user sends an extremely long buffer with no newlines:
		// current behaviour would discard all of it, until they provide a '\n':
		// Only after that '\n' it would start to process bytes as a candidate message.
		// Is that good?
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
