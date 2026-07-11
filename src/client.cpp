#include <iostream>
#include <cstring> // std::strerror()
#include <cerrno>
#include <string_view>
#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/parser.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"

bool	recv_from_client(t_IRC_Server &server, int fd)
{
	static char		buf[t_parser::buf_size];
	t_IRC_Client	&client = server.clients[fd]; // reference to the observed client

	ssize_t	received = recv(fd, buf, sizeof(buf), 0);
	if (received == 0)
		return true; // EOF -> disconnect
	if (received < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return false; // no data ready -> try later
		log_error(strerror(errno), "recv", __FILE__, __LINE__);
		return true; // error -> disconnect
	}

	if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG))
		handle_message_to_discard(client, buf, received);
	else
		client.received_message_buffer.append(buf, received);

	return false;
}

void	handle_client_message(t_IRC_Client &client, t_IRC_Server &server)
{
	std::string	&buf = client.received_message_buffer;
	size_t		pos = std::string::npos;

	while((pos = buf.find_first_of(std::string_view{"\n\0", 2})) != std::string::npos)
	{
		// client may exit but still have pending messages sent:
		// these should be ignored
		if (requested_shutdown || is_flag_set(client.state, t_IRC_Client::DISCONNECT))
			return;

		if (buf[pos] == '\0') // a null-terminator was found in the message, unexpected
		{
			// handle the rest of the malformed message accordingly,
			// silently ignoring the message up to the next newline
			pos = buf.find('\n', pos);
			if (pos == std::string::npos)
			{
				client.state |= t_IRC_Client::DISCARD_MSG;
				buf.clear();
				return;
			}
			else
			{
				buf.erase(0, pos + 1);
				continue;
			}
		}

		if (parse_message(pos, buf, client) == -1)
			build_ERR_INPUTTOOLONG(client);

		if (is_flag_set(client.state, t_IRC_Client::DISCARD_MSG)) // no need to dispatch
			client.state &= ~t_IRC_Client::DISCARD_MSG;
		else
			dispatch_client_command(client, server);

		buf.erase(0, pos + 1);
	}
	if (buf.length() >= t_parser::buf_size)
	{
		build_ERR_INPUTTOOLONG(client);
		client.state |= t_IRC_Client::DISCARD_MSG;
		buf.clear();
		/* NOTE: If user sends an extremely long buffer with no newlines:
		/ current behaviour would discard all of it, until they provide a '\n':
		/ Only after that '\n' it would start to process bytes, looking for a
		/ candidate message. */
	}
}

void	disconnect_client(t_IRC_Server &server, int fd)
{
	// Drop the client's raw pointer from every channel it belongs to before the
	// t_IRC_Client object is destroyed, otherwise those pointers would dangle.
	// disconnect_client is only called for fds already in server.clients
	t_IRC_Client	&client = server.clients[fd];
	for (t_IRC_Channel *channel : client.joined_channels)
	{
		channel->members.erase(&client);
		channel->invited.erase(&client);
		if (channel->members.empty())
			server.channels.erase(channel->name);
	}
	for (auto &[name, channel] : server.channels)
		channel.invited.erase(&client);
	close(fd);
	server.clients.erase(fd);
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });
	std::cout << "Client disconnected.\n";
	std::cout << "total number of clients: " << server.clients.size() << std::endl;
}
