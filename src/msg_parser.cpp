
// FIXME: On linux, when running netcat (or is it only with 'netcat' but not
// with 'nc'?), and inputting something and then ctrl + d twice in a row just
// completely seems to break the server? This needs more testing. Is it SIGPIPE?

// WARN: Should the parser check for null-terminators - that are not 'supposed'
// to be sent by an IRC client, but that may still be sent, as part of an attack
// for example?

#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

#include <string>
#include <iostream>

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

// WARN: client might be labeled 'const' here? Right now yes, but later on probably not,
// as tokenization might change it.
void	prepare_and_parse_message(const size_t pos, std::string &buf, t_IRC_Client &client)
{
	if (pos >= t_parser::buf_size)
	{
		// Too long message detected. Handling:
		// - communicate: ERR_INPUTTOOLONG (417)
		// - no need to set DISCARD_MSG flag here: next mesage may be a candidate
		// - erase from buffer until after the newline (happpens at caller)

		std::cerr << "ERROR 417: ERR_INPUTTOOLONG!" << std::endl; // WARN: just temporary debugger
		return;
	}

	// scan for carriage return (IRC message may end with '\n' or "\r\n")
	bool	has_carriage_return = 0;
	if (pos >= 1 && buf[pos - 1] == '\r')
		has_carriage_return = 1;

	// Check whether client sent an empty message:
	// such a message is to be ignored, according to the IRC protocol.
	if (!pos || (pos == 1 && has_carriage_return))
		return ;

	// Candidate message detected: send string_view for parsing.
	tokenize_message(client, std::string_view{&buf[0], pos - has_carriage_return});
	return ;
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
		std::cerr << "ERROR 417: ERR_INPUTTOOLONG!" << std::endl; // WARN: just temporary debugger
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

// TODO: Refer to the Modern IRC docs, section: "Client-to-Server Protocol Structure"!
// FIXME: Can messages contain more than one space between tokens?
void	tokenize_message(const t_IRC_Client &client, const std::string_view &msg)
{
	// TODO: Tokenization / parsing happens here.
	std::cout	<< "Received from " << client.fd << " : " << msg << std::endl; // WARN: just debugging.

}


// ===================== CLIENT AUTHENTIFICATION CODE ==========================
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

// TODO: Consider cases of commands that are received when they are not valid
// anymore - for example, a nickname has to be set before

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

// FIXME: Should we accept multiple connections from the same client (perhaps there
// is a way for them to connect once, and then somehow change their nickname/name to a
// valid one, and then they could be still validated again? There is a way to check
// the IP address via the socket address info struct of the client.... but is it necessary?)
