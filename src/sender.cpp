#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"
#include "../lib/channel.hpp"
#include "../lib/message_logger.hpp"

#include <sys/socket.h> // for send()
#include <vector> // for the poll_fds vector
#include <string>
#include <cstring> // for std::strerror()
#include <cerrno>

static void	update_send_buffer_and_offset(std::string &send_buf, size_t &offset,
                const size_t sent_bytes);

void	send_messages_to_all_clients(t_IRC_Server &server)
{
	for (size_t i = 0; !requested_shutdown && i < server.poll_fds.size(); )
	{
		if (server.listen_fd == server.poll_fds[i].fd)
		{
			++i;
			continue;
		}
		int				current_fd = server.poll_fds[i].fd;
		t_IRC_Client	&client = server.clients.at(current_fd);
		std::string		&send_buf = client.send_message_buffer;

		if (client.send_offset >= send_buf.size()) // no bytes to send to this client
		{
			if (is_flag_set(client.state, t_IRC_Client::DISCONNECT))
				disconnect_client(server, client.fd);
			else
				++i;
			continue;
		}
		if (is_flag_set(server.poll_fds[i].revents, POLLOUT)) // check whether the fd is available for writing.
		{
			ssize_t	ret = send(client.fd, send_buf.c_str() + client.send_offset,
				 send_buf.size() - client.send_offset, MSG_NOSIGNAL);
			if (ret <= 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					/* This occurs for non-blocking sockets, if space is not
					 * available at the sending socket to hold the message to
					 * be transmitted. -> try again later */
					++i;
				}
				else
				{
					/* errno might be set to EPIPE here.
					* The MSG_NOSIGNAL flag tells send() to avoid writing to
					* the socket if it is broken - which would trigger SIGPIPE
					* and terminate the program, in case that signal is not
					* handled by a custom signal handler.
					* Instead, thanks to MSG_NOSIGNAL, send() returns -1 and
					* sets errno to EPIPE.

					* Other failures are possible here, but they all mean bad
					* things for the connection -> time to disconnect client */
					log_error(std::strerror(errno), "send", __FILE__, __LINE__);
					broadcast_non_requested_disconnect_msg(client);
					disconnect_client(server, client.fd);
				}
				continue;
			}
			message_logger(std::string_view(send_buf.data() + client.send_offset,
					static_cast<size_t>(ret)), "->", client);
			update_send_buffer_and_offset(send_buf, client.send_offset, ret);
		}
		if (client.send_offset < send_buf.size()) // indicate that we need to write to the client
			server.poll_fds[i].events |= POLLOUT;
		else
			server.poll_fds[i].events &= ~POLLOUT;

		if (is_flag_set(client.state, t_IRC_Client::DISCONNECT)
			&& client.send_offset >= send_buf.size())
		{
			disconnect_client(server, client.fd);
			continue;
		}
		++i;
	}
}

// std::string's erase() and clear() may throw std::bad_alloc
static void	update_send_buffer_and_offset(std::string &send_buf, size_t &offset,
                const size_t sent_bytes)
{
	size_t	buf_size = send_buf.size();

	offset += sent_bytes;
	if (offset >= buf_size)
	{
		send_buf.clear();
		offset = 0;
	}
	else if (offset >= t_IRC_Client::offset_threshold && offset >= buf_size / 2)
	{
		send_buf.erase(0, offset);
		offset = 0;
	}
}
