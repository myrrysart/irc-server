
// WARN: does PASS need to be implemented?
// Is that the same password as the one that is supposed to be provided when
// running the irc-server (./ircserv port PASSWORD)

// TODO: Consider cases of commands that are received when they are not valid
// anymore - for example, a nickname has to be set before

// TODO: test with netcat (nc)

#include "../lib/irc_fatstruct.hpp"

#include <string>

// TODO: ONLY PUT A TIMER FOR KICKING OUT THE USER DURING THEIR REGISTRATION PHASE:
// Do not kick them out for being inactive after they have registered!
// date: 26.05.2026

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

// NOTE: expect to receive a string 'message' that is everything up to the '\n'
// or "\r\n" - non inclusive!!
// TODO: IF THE USER IS IDLE VERY LONG TIME, KICK THEM OUT!!!
//
void	parse_input(const std::string &message, t_IRC_Server &IRC_Server,
			const std::size_t i) // NOTE: i is the index of the concerned client.
{
	// authenticate user - should not be part of a loop, only a check for the
	// correct bit flag at the top before entering it...

	std::size_t	 j;

	if (IRC_Server.clients.i.state & ERROR_FLAG)
		j = 0;
	else if (IRC_Server.clients.i.state & IS_OK)
		j = 1;
	else
	{
		if (!(IRC_Server.clients.i.state & PASSWORD))


	}





	if (!(IRC_Server.clients.i.state & IS_OK))
		authenticate_user();





}

void	authenticate_user(const std::string &message, t_IRC_Server &IRC_Server,
			const std::size_t i)
{
	


}
