
#include "../lib/irc_fatstruct.hpp"

#include <string_view>
#include <iostream>


// NOTE: "Clients MUST NOT include a source when sending a message. Servers MAY
// include a source on any message, and MAY leave a source off of any message.
// Clients MUST be able to process any given message the same way whether it
// contains a source or does not contain one."

// TODO: "When sending messages, ensure that a pair of \r\n characters follows
// every single message your software sends out"

// NOTE: "If a command is sent from a client to a server with less parameters
// than the command requires to be processed, the server will reply with an
// ERR_NEEDMOREPARAMS (461) numeric and the command will fail."
// TODO:
// ERR_NEEDMOREPARAMS (461)
//   "<client> <command> :Not enough parameters"
// Returned when a client command cannot be parsed because not enough parameters
// were supplied. The text used in the last param of this message may vary.
// TODO: send message instead.
void	send_ERR_NEEDMOREPARAMS(const t_IRC_Client &client)
{
	std::string_view	capitalized_verb{client.parser.verb_in_caps,
		                    client.parser.verb.size()};
	// WARN: temporary solution. Also replace "<client>" with the right thing?
	std::cout
		<< "<client> " << client.nick << ' '
		<< capitalized_verb << " :Not enough parameters\n";
}

// TODO:
// ERR_ALREADYREGISTERED (462)
// "<client> :You may not reregister"
// Returned when a client tries to change a detail that can only be set during
// registration (such as resending the PASS or USER after registration). The text
// used in the last param of this message varies.
// TODO: send message instead.
void	send_ERR_ALREADYREGISTERED(const t_IRC_Client &client)
{
	// WARN: temporary solution. Also replace "<client>" with the right thing?
	std::cout << "<client> " << client.nick << ":You may not reregister\n";
}


// TODO:
// ERR_PASSWDMISMATCH (464)
//   "<client> :Password incorrect"
// Returned to indicate that the connection could not be registered as the
// password was either incorrect or not supplied. The text used in the last param
// of this message may vary.
// TODO: send message instead.
void	send_ERR_PASSWDMISMATCH(const t_IRC_Client &client)
{
	// WARN: temporary solution. Also replace "<client>" with the right thing?
		std::cout << "<client> " << client.nick << ":Password incorrect\n";
}


