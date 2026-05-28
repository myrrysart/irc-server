
// TODO: Consider cases of commands that are received when they are not valid
// anymore - for example, a nickname has to be set before

// TODO: test with netcat (nc)

#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

#include <string>
#include <iostream>

void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
	                              const ssize_t received)
{
	std::string	&msg = client.received_message_buffer;
	ssize_t		pos = 0;

	while (pos < received && pos != '\n')
		++pos;

	if (pos < received)
	{
		// newline has been found in the buffer.
		// append the trailing part after the newline
		if (pos < received - 1) // otherwise, there is nothing to append.
			msg.append(buf[pos + 1], received - pos - 1);
		// unset DISCARD_MSG flag
		client.state &= ~t_IRC_Client::Flags::DISCARD_MSG;
	}
	// else: no newline in the received buffer; the buffer can be igonored,
	// and DISCARD_MSG flag should not be unset, since the next batch should
	// still be discarded.
	// NOTE: the error message ERR_INPUTTOOLONG (417) should have been
	// communicated to the client earlier.
	// WARN: code should return false in this case - make sure it is the case
	// after the call to this function.
}

int	prepare_and_parse_message(const size_t pos, std::string &buf, t_IRC_Client &client)
{
	if (pos >= t_parser::buf_size)
	{
	// handle the too long message error:
		// - communicate: ERR_INPUTTOOLONG (417)
		// - set DISCARD_MSG flag so that you can discard the rest of incoming message
		// - erase the msg buffer

		// communicate: ERR_INPUTTOOLONG (417)
		std::cerr << "ERROR 417: ERR_INPUTTOOLONG!" << std::endl;    // WARN: just temporary debugger
		// erase from buffer everything until (and including) the newline
		// NOTE: no need to set DISCARD_MSG flag here: next mesage may be a candidate
		buf.erase(0, pos + 1);
		return (-1);	// start again at the top of the looop:
						// remaining bytes might hold a valid message.
	}

	// scan for carriage return
	bool	has_carriage_return = 0;
	if (pos >= 1 && buf[pos - 1] == '\r')
		has_carriage_return = 1;

	// candidate message detected: send string_view for parsing.
	tokenize_message(client, std::string_view{&buf[0], pos - has_carriage_return});
	return (0);
}

void	check_for_too_long_message(std::string &buf, t_IRC_Client &client)
{
	// when execution arrives here:
	// - 'buf' holds only the trailing, non processed bytes. It does not contain
	// a newline. But it can still hold a message larger than 512 bytes! And its
	// message might not be ready!
	if (buf.length() >= t_parser::buf_size)
	{
		// communicate: ERR_INPUTTOOLONG (417)
		std::cerr << "ERROR 417: ERR_INPUTTOOLONG!" << std::endl;    // WARN: just temporary debugger
		// set DISCARD_MSG flag
		client.state |= t_IRC_Client::Flags::DISCARD_MSG;
		// discard whole buffer
		buf.clear();

		/* NOTE: If user sends an extremely long buffer with no newlines:
		/ current behaviour would discard all of it, until they provide a '\n':
		/ Only after that '\n' it would start to process bytes, looking for a
		/ candidate message. Is the team on board with this plan? */
	}
}

// FIXME: Can messages contain more than one space between tokens?
void	tokenize_message(const t_IRC_Client &client, const std::string_view &msg)
{
	// TODO: Tokenization / parsing happens here.
	std::cout	<< "Received from " << client.fd << " : " << msg << std::endl; // WARN: just debugging.

}

int8_t	prepare_message_for_parsing(const size_t pos, std::string &buf)
{
	if (pos >= t_parser::buf_size)
	{
	// handle the error:
		// - communicate: ERR_INPUTTOOLONG (417)
		// - set DISCARD_MSG flag so that you can discard the rest of incoming message
		// - erase the msg buffer

		// communicate: ERR_INPUTTOOLONG (417)
		std::cerr << "ERROR 417: ERR_INPUTTOOLONG!" << std::endl;    // WARN: just temporary debugger
		// erase from buffer everything until (and including) the newline
		// NOTE: no need to set DISCARD_MSG flag here: next mesage may be a candidate
		buf.erase(0, pos + 1);
		return (-1); // start again: remaining bytes might hold a valid message.
		// continue;
	}

	bool	has_carriage_return = 0;
	// scan for carriage return
	if (pos >= 1 && buf[pos - 1] == '\r')
		has_carriage_return = 1;

	return (has_carriage_return);
}





// TODO: IF THE USER IS IDLE VERY LONG TIME, KICK THEM OUT!!!
// BUT ONLY DO THAT DURING THEIR REGISTRATION PHASE:
// Do not kick them out for being inactive after they have registered!

// TODO: FIRST THING TO DO: TOKENIZATION!
// Create an array of std::string_views:
// 		- 0'th index = COMMAND
// 		- 1'st index onwards = arguments, one word per string_view
// 		- Very last cell = (:trailing) - but perhaps without the ':'? , optional,
// 			and contains all of its words, unsplitted. Refer to "Message Parsing
// 			and Assembly" on the modern Horse docs: https://modern.ircdocs.horse/#client-messages

// TODO: store somewhere an array of constexpr commands, with all the available
// commands that we are supposed to implement: The parser would compare against
// these once the 'tokenization' process will be done.

/*
 * NOTE: CLIENT AUTHENTIFICATION: unused draft, probably deprecated
void	parse_input(const std::string &message, t_IRC_Server &IRC_Server,
			const size_t i) // NOTE: i is the index of the concerned client.
{
	// authenticate user - should not be part of a loop, only a check for the
	// correct bit flag at the top before entering it...

	size_t	 j;

	if (IRC_Server.clients.i.state & ERROR_FLAG)
		j = 0;
	else if (IRC_Server.clients.i.state & IS_OK)
		j = 1;
	else
	{
		if (!(IRC_Server.clients.i.state & PASSWORD))


	}





	if (!(IRC_Server.clients.i.state & IS_OK))
		authenticate_client();



}

void	authenticate_client(const std::string &message, t_IRC_Server &IRC_Server,
			const size_t i)
{
	


}
*/
