
#include "../lib/irc_fatstruct.hpp"

#include <string_view>
#include <iostream>

// FIXME: Consider designing a single function that could be overloaded and serve
// all of the present functions.
// If that ends up making sense, we could design an enum with the macros of the
// ERRORS, and an array of static constexpr const char * strings, where the enum
// values would match the right indices for their appropriate string - since
// often these numeric replies have a string message as their trailing parameter.
// This might not make sense for certain numeric replies, which have to output
// more complex strings which include variables... In which case, std::string
// would be useful - but that would add a lot of dynamic memory allocation overhead

// NOTE: "Clients MUST NOT include a source when sending a message. Servers MAY
// include a source on any message, and MAY leave a source off of any message.
// Clients MUST be able to process any given message the same way whether it
// contains a source or does not contain one."

// TODO: "When sending messages, ensure that a pair of \r\n characters follows
// every single message your software sends out"

// NOTE: When IRC documentation uses the '<client>' placeholder, it should be
// replaced by the client's nickname

// TODO:
// ERR_NONICKNAMEGIVEN (431)
// "<client> :No nickname given"
// "Returned when a nickname parameter is expected for a command but isn’t given."
void	send_ERR_NONICKNAMEGIVEN(const t_IRC_Client &client)
{
	// WARN: temporary solution, send instead.
	std::cout << client.nick << " :No nickname given\n";
}

// TODO:
// ERR_ERRONEOUSNICKNAME (432)
// "<client> <nick> :Erroneus nickname"
// Returned when a NICK command cannot be successfully completed as the desired
// nickname contains characters that are disallowed by the server. See the NICK
// command for more information on characters which are allowed in various IRC
// servers. The text used in the last param of this message may vary."
void	send_ERR_ERONEOUSNICKNAME(const t_IRC_Client &client,
            const std::string_view &new_nick)
{
	// WARN: temporary solution, send instead
	std::cout
		<< client.nick << ' ' << new_nick
		<< " :Erroneous nickname. Accepted characters: alphabetical letters, "
			"digits, and the following symbols: \"[]{}\\|#&:$%<>_-\". "
			"First characters may not be: a digit, '#', ':' or \"&#\". "
			"Only the first 30 characters will be considered.\n";
	// WARN: is this message too long? Check IRC documentation about length of
	// server-client messages.
	// WARN: update all allowed symbols if they change!
}

// TODO:
// ERR_NICKNAMEINUSE (433)
//    "<client> <nick> :Nickname is already in use"
// Returned when a NICK command cannot be successfully completed as the desired
// nickname is already in use on the network. The text used in the last param of
// this message may vary."
void	send_ERR_NICKNAMEINUSE(const t_IRC_Client &client,
            const std::string_view &new_nick)
{
	// WARN: temporary solution
	// FIXME: Once the string to send is constructed: new_nick could probably be
	// already deprecated! It is a string_view that would become invalid!!!
	std::cout << client.nick << ' ' << new_nick << " :Nickname is already in use\n";
}

// NOTE: "If a command is sent from a client to a server with less parameters
// than the command requires to be processed, the server will reply with an
// ERR_NEEDMOREPARAMS (461) numeric and the command will fail."
// TODO:
// ERR_NEEDMOREPARAMS (461)
//   "<client> <command> :Not enough parameters"
// Returned when a client command cannot be parsed because not enough parameters
// were supplied. The text used in the last param of this message may vary."
// TODO: send message instead.
void	send_ERR_NEEDMOREPARAMS(const t_IRC_Client &client)
{
	std::string_view	capitalized_verb{client.parser.verb_in_caps,
                                         client.parser.verb.size()};
	// WARN: temporary solution
	std::cout
		<< client.nick << ' ' << capitalized_verb << " :Not enough parameters\n";
}


// TODO:
// ERR_ALREADYREGISTERED (462)
// "<client> :You may not reregister"
// Returned when a client tries to change a detail that can only be set during
// registration (such as resending the PASS or USER after registration). The text
// used in the last param of this message varies."
// TODO: send message instead.
void	send_ERR_ALREADYREGISTERED(const t_IRC_Client &client)
{
	// WARN: temporary solution
	std::cout << client.nick << " :You may not reregister\n";
}

// TODO:
// ERR_PASSWDMISMATCH (464)
//   "<client> :Password incorrect"
// Returned to indicate that the connection could not be registered as the
// password was either incorrect or not supplied. The text used in the last param
// of this message may vary."
// TODO: send message instead.
void	send_ERR_PASSWDMISMATCH(const t_IRC_Client &client)
{
	// WARN: temporary solution
	std::cout << client.nick << " :Password incorrect\n";
}
