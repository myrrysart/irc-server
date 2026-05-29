
#include "../lib/irc_fatstruct.hpp"

#include <iostream>
#include <string_view>

// FIXME: Should we accept multiple connections from the same client (perhaps there
// is a way for them to connect once, and then somehow change their nickname/name to a
// valid one, and then they could be still validated again? There is a way to check
// the IP address via the socket address info struct of the client.... but is it necessary?)

// ======================== CLIENT REGISTRATION ================================
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


// TODO: "When sending messages, ensure that a pair of \r\n characters follows every single message your software sends out"

// TODO: Refer to the Modern IRC docs, section: "Client-to-Server Protocol Structure"!

// NOTE: "If a command is sent from a client to a server with less parameters
// than the command requires to be processed, the server will reply with an
// ERR_NEEDMOREPARAMS (461) numeric and the command will fail."

// NOTE: "Clients MUST NOT include a source when sending a message. Servers MAY
// include a source on any message, and MAY leave a source off of any message.
// Clients MUST be able to process any given message the same way whether it
// contains a source or does not contain one."

// TODO: When a messsage is unknown: Irssi at least returns : "Unknown command: <the_command>"

// TODO: NICK: cap nicknames at 30 characters, and trim any characters beyond that
// without saying anything. Example from the old Horse docs:
// 'dan-is-my-name-dont-wear-it-out-at-all' became: 'dan-is-my-name-dont-wear-it-ou'
// NOTE: In this example on Horse, the returned welcome 001 message does not
// end with the nickname at all, unlike other, shorter examples?!


// NOTE: "Message parts and parameters are separated by one or more ASCII SPACE characters"
void	tokenize_message(const t_IRC_Client &client, const std::string_view &msg)
{
	// TODO: Tokenization / parsing happens here.
	std::cout	<< "Received from " << client.fd << " : " << msg << std::endl; // WARN: just debugging.

}




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
