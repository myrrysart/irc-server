


// FIXME: Should we accept multiple connections from the same client (perhaps there
// is a way for them to connect once, and then somehow change their nickname/name to a
// valid one, and then they could be still validated again? There is a way to check
// the IP address via the socket address info struct of the client.... but is it necessary?)

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
