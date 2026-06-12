
#include "../lib/irc_fatstruct.hpp"

#include <string>
#include <string_view>

static void	append_common_reply_prefix(std::string &buffer, const char *numeric,
                const std::string_view nick);

// NOTE: From the modern documentation, regarding numeric replies:
// "Most messages sent from a client to a server generates a reply of some sort.
// The most common form of reply is the numeric reply, used for both errors and
// normal replies. Distinct from a normal message, a numeric reply MUST contain
// a <source> and use a three-digit numeric as the command. A numeric reply
// SHOULD contain the target of the reply as the first parameter of the message.
// A numeric reply is not allowed to originate from a client.
// In all other respects, a numeric reply is just like a normal message. A list
// of numeric replies is supplied in the Numerics section."

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

// WARN: These numeric replies heavily rely on std::string internals, which,
// in theory, could throw exceptions when they allocate memory. Therefore,
// always make sure that a try-catch block is handling the eventual throw,
// when calling any of these - setting the appropriate server flag.

// RPL_WELCOME (001)
// "<client> :Welcome to the <networkname> Network, <nick>[!<user>@<host>]"
// "The first message sent after client registration, this message introduces the
// client to the network. The text used in the last param of this message varies wildly.
// Servers that implement spoofed hostmasks in any capacity SHOULD NOT include the
// extended (complete) hostmask in the last parameter of this reply, either for
// all clients or for those whose hostnames have been spoofed. This is because
// some clients try to extract the hostname from this final parameter of this
// message and resolve this hostname, in order to discover their ‘local IP address’.
// Clients MUST NOT try to extract the hostname from the final parameter of this
// message and then attempt to resolve this hostname. This method of operation
// WILL BREAK and will cause issues when the server returns a spoofed hostname."
void	build_RPL_WELCOME(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "001", client.nick);
	buffer += ":Welcome to this Helsinki based Internet Relay Chat server, ";
	buffer += client.nick;
	buffer += '!';
	buffer += client.username;
	buffer += '@';
	buffer += client.hostname;
	buffer += "\r\n";
}

// RPL_YOURHOST (002)
// "<client> :Your host is <servername>, running version <version>"
// "Part of the post-registration greeting, this numeric returns the name and
// software/version of the server the client is currently connected to. The text
// used in the last param of this message varies wildly."
void	build_RPL_YOURHOST(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "002", client.nick);
	buffer += ":Your host is ";
	buffer += t_IRC_Server::name;
	buffer += ", running version ";
	buffer += t_IRC_Server::version;
	buffer += "\r\n";
}

// ERR_INPUTTOOLONG (417)
// "<client> :Input line was too long"
// "Indicates a given line does not follow the specified size limits (512 bytes
// for the main section, 4094 or 8191 bytes for the tag section)."
void	build_ERR_INPUTTOOLONG(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "417", client.nick);
	buffer += ":Input line was too long\r\n";
}

// WARN: If the message is extremely long but still within the 512 byte cap,
// and there are no spaces: the command will be huge. Does the protocol allow
// to send such long messages to the client? Research this.
// ERR_UNKNOWNCOMMAND (421)
// "<client> <command> :Unknown command"
// "Sent to a registered client to indicate that the command they sent isn’t
// known by the server. The text used in the last param of this message may vary."
void	build_ERR_UNKNOWNCOMMAND(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "421", client.nick);
	buffer += client.parser.verb;
	buffer += " :Unknown command\r\n";
}


// ERR_NONICKNAMEGIVEN (431)
// "<client> :No nickname given"
// "Returned when a nickname parameter is expected for a command but isn’t given."
void	build_ERR_NONICKNAMEGIVEN(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "431", client.nick);
	buffer += ":No nickname given\r\n";
}

// ERR_ERRONEOUSNICKNAME (432)
// "<client> <nick> :Erroneus nickname"
// Returned when a NICK command cannot be successfully completed as the desired
// nickname contains characters that are disallowed by the server. See the NICK
// command for more information on characters which are allowed in various IRC
// servers. The text used in the last param of this message may vary."
void	build_ERR_ERRONEOUSNICKNAME(t_IRC_Client &client,
            const std::string_view new_nick)
{
	// WARN: is the last parameter too long? Check IRC documentation regarding
	// length of server-client messages.
	// WARN: update all allowed symbols if they change!

	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "432", client.nick);
	buffer += new_nick;
	buffer += " :Erroneous nickname. Accepted characters: alphabetical "
		"letters, digits, and the following symbols: \"[]{}\\|#&:$%<>_-\". "
		"First characters may not be: a digit, '#', ':' or \"&#\". "
		"Only the first 30 characters will be considered.\r\n";
}

// ERR_NICKNAMEINUSE (433)
//    "<client> <nick> :Nickname is already in use"
// Returned when a NICK command cannot be successfully completed as the desired
// nickname is already in use on the network. The text used in the last param of
// this message may vary."
void	build_ERR_NICKNAMEINUSE(t_IRC_Client &client,
            const std::string_view new_nick)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "433", client.nick);
	buffer += new_nick;
	buffer += " :Nickname is already in use\r\n";

}

// ERR_NOTREGISTERED (451)
// "<client> :You have not registered"
// "Returned when a client command cannot be parsed as they are not yet registered.
// Servers offer only a limited subset of commands until clients are properly
// registered to the server. The text used in the last param of this message may vary."
void	build_ERR_NOTREGISTERED(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "451", client.nick);
	buffer += ":You have not registered\r\n";
}

// ERR_NEEDMOREPARAMS (461)
// "<client> <command> :Not enough parameters"
// "Returned when a client command cannot be parsed because not enough parameters
// were supplied. The text used in the last param of this message may vary."
// WARN: Make sure that verb_in_caps is not deprecated when this function is
// called! It is a static character array, shared between all clients! But it
// should be fine, since concatenating this message to the output buffer happens
// right after parsing the incoming message.
void	build_ERR_NEEDMOREPARAMS(t_IRC_Client &client)
{
	std::string_view	capitalized_verb{client.parser.verb_in_caps,
                                         client.parser.verb.size()};
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "461", client.nick);
	buffer += capitalized_verb;
	buffer += " :Not enough parameters\r\n";
}


// ERR_ALREADYREGISTERED (462)
// "<client> :You may not reregister"
// "Returned when a client tries to change a detail that can only be set during
// registration (such as resending the PASS or USER after registration). The text
// used in the last param of this message varies."
void	build_ERR_ALREADYREGISTERED(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "462", client.nick);
	buffer += ":You may not reregister\r\n";
}

// FIXME: the client gets disconnected BEFORE getting this message!
// Make sure they receive it and only then send it !!

// ERR_PASSWDMISMATCH (464)
// "<client> :Password incorrect"
// "Returned to indicate that the connection could not be registered as the
// password was either incorrect or not supplied. The text used in the last param
// of this message may vary."
void	build_ERR_PASSWDMISMATCH(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "464", client.nick);
	buffer += ":Password incorrect\r\n";
}

void	append_common_reply_prefix(std::string &buffer, const char *numeric,
            const std::string_view nick)
{
	buffer += ':';
	buffer += (t_IRC_Server::name);
	buffer += ' ';
	buffer += numeric;
	buffer += ' ';
	buffer += nick;
	buffer += ' ';
}
