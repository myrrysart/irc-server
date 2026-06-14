#include "../lib/irc_fatstruct.hpp"
#include "../lib/server.hpp"

#include <sys/socket.h>
#include <unistd.h> // for close()
#include <unordered_map>
#include <vector> // for std::erase_if()
#include <string>
#include <cstring> // for std::strerror()
#include <cerrno>
#include <iostream>
#include <exception>

static void	disconnect_client_during_iterator_walk(t_IRC_Server &server,
                std::unordered_map<int, t_IRC_Client>::iterator &iterator);
static void	update_send_buffer_and_offset(std::string &send_buf, size_t &offset,
                const size_t sent_bytes);

void	send_messages_to_all_clients(t_IRC_Server &server)
{
	ssize_t	ret;

	for (std::unordered_map<int, t_IRC_Client>::iterator iterator = server.clients.begin();
		iterator != server.clients.end() && !requested_shutdown; )
	{
		t_IRC_Client	&client = iterator->second;
		std::string		&send_buf = client.send_message_buffer;
		if (client.send_offset >= send_buf.size()) // no bytes to send to this client
		{
			if (!is_flag_set(client.state, t_IRC_Client::DISCONNECT))
				++iterator;
			else
				disconnect_client_during_iterator_walk(server, iterator);
			continue;
		}
		ret = send(client.fd, send_buf.c_str() + client.send_offset,
			 send_buf.size() - client.send_offset, 0);
		if (ret == -1)
		{
			log_error(std::strerror(errno), __FILE__, __LINE__, 0);
			if (errno == EPIPE) // WARN: for this to work, signal handler should ignore SIGPIPE, or the sockets should use some flag for it
			{
				// FIXME: SIGPIPE is not being ignored right now, so if a SIGPIPE
				// occurs during send(), the program would terminate and would
				// not be able to check the return value of send().
				// SIGPIPE is a signal triggered by writing to a broken pipe/socket
				// if that signal is ignored (either by the global signal handler
				// or by some socket flags), the kernel instead reports the error via:
				// send() → -1
				// errno = EPIPE
				disconnect_client_during_iterator_walk(server, iterator);
				continue;
			}
			else
			{
				requested_shutdown = 1;
				return;
			}
		}
		// WARN: Is it 100% OK not to handle ret == 0?
		try {
			update_send_buffer_and_offset(send_buf, client.send_offset, ret);
		} catch (const std::exception &e) {
			log_error(e.what(), __FILE__, __LINE__, 1);
			// WARN: Should an error message be sent to all clients? This exception
			// is most probably an std::bad_alloc, which is fatal...
			requested_shutdown = 1;
			return;
		}
		if (is_flag_set(client.state, t_IRC_Client::DISCONNECT)
			&& client.send_offset >= send_buf.size())
		{
			disconnect_client_during_iterator_walk(server, iterator);
			continue;
		}
		++iterator;
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

// WARN: Should check if this function is working as intended...
static void	disconnect_client_during_iterator_walk(t_IRC_Server &server,
                std::unordered_map<int, t_IRC_Client>::iterator &iterator)
{
	int	fd = iterator->second.fd;
	// WARN: handle close() failure?
	close(fd);

	// erases the client object from the unordered_map, but returns and assigns
	// the next valid iterator for the map
	iterator = server.clients.erase(iterator);

	// cleans up the corresponding poll_fd key in the vector
	std::erase_if(server.poll_fds, [fd](const pollfd& pfd){ return pfd.fd == fd; });

	std::cout
		<< "Client disconnected.\n"
		"total number of clients: " << server.clients.size()
		<< std::endl;
}
