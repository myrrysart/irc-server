
// FIXME: On linux, when running netcat (or is it only with 'netcat' but not
// with 'nc'?), and inputting something and then ctrl + d twice in a row just
// completely seems to break the server? This needs more testing. Is it SIGPIPE?

// WARN: Should the parser check for null-terminators - that are not 'supposed'
// to be sent by an IRC client, but that may still be sent, as part of an attack
// for example?

#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"
#include "../lib/numerics.hpp"

#include <iostream>
#include <string>
#include <string_view>

/* out-of-line zero initialization of shared static buffer in t_parser struct */
char	t_parser::verb_in_caps[t_parser::longest_cmd_size];

int	prepare_and_parse_message(const size_t pos, const std::string &buf, t_IRC_Client &client)
{
	if (pos >= t_parser::buf_size)
	{
		// Too long message detected. Handling:
		// - communicate: ERR_INPUTTOOLONG (417)
		// - erase from buffer until after the newline (happpens at caller)

		// FIXME: adjust when ready - this next block does not make much sense in the
		// grand scheme of things, try to centralize this with the rest of send loop.
		// Especially clearing the buffer does not make sense: it might still hold
		// previous partially unsent messages!
		// WARN: The INPUTTOOLONG reply should be sent. But if there is a queue
		// system, shouldn't all messages be sent from some centeralized section of the code?
		if (build_ERR_INPUTTOOLONG(client) == -1)
			return (-1);
		if (send(client.fd, client.send_message_buffer.c_str(),
			client.send_message_buffer.size(), 0) == -1)
		{
			// FIXME: handle the error. Many failure possibilities here...



		}
		client.send_message_buffer.clear();
		client.state |= t_IRC_Client::DISCARD_MSG;

		return (0);
	}

	// scan for carriage return (IRC message may end with '\n' or "\r\n")
	bool	has_carriage_return = 0;
	if (pos >= 1 && buf[pos - 1] == '\r')
		has_carriage_return = 1;

	// Check whether client sent an empty message:
	// such a message is to be ignored, according to the IRC protocol.
	if (!pos || (pos == 1 && has_carriage_return))
	{
		client.state |= t_IRC_Client::DISCARD_MSG;
		return (0);
	}

	// Candidate message detected: send string_view for parsing.
	tokenize_message(client, std::string_view{&buf[0], pos - has_carriage_return});
	return (0);
}

void	tokenize_message(t_IRC_Client &client, const std::string_view &msg)
{
	size_t	i = 0;
	size_t	j = 0;

	// WARN: just debugging:
	std::cout	<< "Received from " << client.fd << " : " << msg << std::endl;

	i = msg.find(' ');
	if (i == std::string_view::npos)
		i = msg.size();
	client.parser.verb = std::string_view{&msg[0], i};

	for (size_t k; i != std::string_view::npos; ++j)
	{
		i = msg.find_first_not_of(' ', i);
		if (i == std::string_view::npos)
			break;
		k = i;
		if (msg[k] == ':') // for the trailing parameter
		{
			++k;
			++i = msg.size();
		}
		else
		{
			i = msg.find(' ', i);
			if (i == std::string_view::npos)
				i = msg.size();
		}
		client.parser.params[j] = std::string_view{&msg[k], i - k};
	}
	client.parser.n_params = j;
}

int	check_for_too_long_message(std::string &buf, t_IRC_Client &client)
{
	// when execution arrives here:
	// - 'buf' holds only the trailing, non processed bytes. It does not contain
	// a newline. But it can still hold a message larger than 512 bytes! And its
	// message might not be ready!
	if (buf.length() >= t_parser::buf_size)
	{
		// communicate: ERR_INPUTTOOLONG (417)
		if (build_ERR_INPUTTOOLONG(client) == -1)
			return (-1);

		// FIXME: adjust when ready - this next block does not make much sense in the
		// grand scheme of things, try to centralize this with the rest of send loop.
		// Especially clearing the buffer does not make sense: it might still hold
		// previous partially unsent messages!
		// WARN: The reply should be sent at this moment. But if there is a queue
		// system, shouldn't all messages be sent from some centeralized section of the code?
		if (send(client.fd, client.send_message_buffer.c_str(),
			client.send_message_buffer.size(), 0) == -1)
		{
			// FIXME: handle the error. Many failure possibilities here...



		}
		client.send_message_buffer.clear();

		// set DISCARD_MSG flag
		client.state |= t_IRC_Client::DISCARD_MSG;

		// discard whole buffer
		buf.clear();

		/* NOTE: If user sends an extremely long buffer with no newlines:
		/ current behaviour would discard all of it, until they provide a '\n':
		/ Only after that '\n' it would start to process bytes, looking for a
		/ candidate message. Is the team on board with this plan? */
	}
	return (0);
}

void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
            const ssize_t received)
{
	std::string	&msg = client.received_message_buffer;
	ssize_t		pos = 0;

	while (pos < received && buf[pos] != '\n')
		++pos;

	if (pos < received)
	{
		// newline has been found in the buffer.
		// append the trailing part after the newline
		if (pos < received - 1) // otherwise, there is nothing to append.
			msg.append(buf[pos + 1], received - pos - 1);
		// unset DISCARD_MSG flag
		client.state &= ~t_IRC_Client::DISCARD_MSG;
	}
	// else: no newline in the received buffer; the buffer can be igonored,
	// the received_message_buffer will remain empty, which will skip the loop
	// in the next function call (handle_client_message()), since no newline
	// would be found.
	// and DISCARD_MSG flag should not be unset, since the next batch should
	// still be discarded.
	// NOTE: the error message ERR_INPUTTOOLONG (417) should have been
	// communicated to the client earlier.
	// WARN: code should return false in this case - make sure it is the case
	// after the call to this function.
}
