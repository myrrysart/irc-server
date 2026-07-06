
#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

#include <string>
#include <string_view>

/* out-of-line zero initialization of shared static buffer in t_parser struct */
char	t_parser::verb_in_caps[t_parser::longest_cmd_size];

int	parse_message(const size_t pos, const std::string &buf, t_IRC_Client &client)
{
	if (pos >= t_parser::buf_size)
	{
		// Too long message detected. Handling:
		// - communicate: ERR_INPUTTOOLONG (417)
		// - erase from buffer until after the newline
		// (Both of these happen at the caller, as of this moment.)

		client.state |= t_IRC_Client::DISCARD_MSG;
		return (-1);
	}

	// scan for carriage return (IRC message may end with '\n' or "\r\n")
	bool	has_cr = 0;
	if (pos >= 1 && buf[pos - 1] == '\r')
		has_cr = 1;

	size_t i = skip_leading_spaces_and_check_for_empty_message(buf, pos, has_cr);
	if (i == std::string::npos)
	{
		client.state |= t_IRC_Client::DISCARD_MSG;
		return (0);
	}

	// Candidate message detected: send first message in buffer for tokenizing
	tokenize_message(client, std::string_view{&buf[i], pos - i - has_cr});
	return (0);
}

void	tokenize_message(t_IRC_Client &client, const std::string_view msg)
{
	size_t	i = 0;
	size_t	j = 0;

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
		{
			ssize_t	len = received - pos - 1;
			msg.append(&buf[pos + 1], static_cast<size_t>(len));
		}

		// unset DISCARD_MSG flag
		client.state &= ~t_IRC_Client::DISCARD_MSG;
	}
	// else: no newline in the received buffer; it can therefore be igonored,
	// 'received_message_buffer' will remain empty, which will skip the loop
	// in the next function call (handle_client_message()), since no newline
	// would be found, at the start of that function.
	// DISCARD_MSG flag should not be unset after this call, since the next
	// batch of received bytes should still be discarded, up to the next newline.
}
