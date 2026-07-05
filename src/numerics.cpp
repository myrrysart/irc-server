
#include "../lib/irc_fatstruct.hpp"

#include <string>
#include <string_view>

void	append_common_reply_prefix(std::string &buffer,
            std::string_view numeric, std::string_view nick);

static void	append_channel_modes(std::string &buffer, const t_IRC_Channel &channel)
{
	std::string	modes;
	if (is_flag_set(channel.mode, INVITE))
		modes += 'i';
	if (is_flag_set(channel.mode, TOPIC))
		modes += 't';
	if (is_flag_set(channel.mode, KEY) && !channel.key.empty())
		modes += 'k';
	if (is_flag_set(channel.mode, LIMIT))
		modes += 'l';
	if (modes.empty())
		return;

	buffer += '+';
	buffer += modes;
	if (is_flag_set(channel.mode, KEY) && !channel.key.empty())
	{
		buffer += ' ';
		buffer += channel.key;
	}
	if (is_flag_set(channel.mode, LIMIT))
	{
		buffer += ' ';
		buffer += std::to_string(channel.user_limit);
	}
}

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
            std::string_view new_nick)
{
	// WARN: is the last parameter too long? Check IRC documentation regarding
	// length of server-client messages.

	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "432", client.nick);
	buffer += new_nick;
	buffer += " :Erroneous nickname. Accepted characters: alphabetical "
		"letters, digits, and the following symbols: \"";
	buffer += t_IRC_Client::allowed_symbols_nick;
	buffer += "\". "
		"First characters may not be: a digit, '#', ':' or \"&#\". "
		"Only the first 30 characters will be considered.\r\n";
}

// ERR_NICKNAMEINUSE (433)
//    "<client> <nick> :Nickname is already in use"
// Returned when a NICK command cannot be successfully completed as the desired
// nickname is already in use on the network. The text used in the last param of
// this message may vary."
void	build_ERR_NICKNAMEINUSE(t_IRC_Client &client,
            std::string_view new_nick)
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

// RPL_UMODEIS (221) "<client> <user modes>"
void	build_RPL_UMODEIS(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;
	append_common_reply_prefix(buffer, "221", client.nick);
	buffer += "+\r\n";
}

// ERR_USERSDONTMATCH (502) "<client> :Cant change mode for other users"
void	build_ERR_USERSDONTMATCH(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;
	append_common_reply_prefix(buffer, "502", client.nick);
	buffer += ":Cant change mode for other users\r\n";
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

// RPL_CREATED (003)
// "<client> :This server was created <date>"
void	build_RPL_CREATED(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "003", client.nick);
	buffer += ":This server was created Mon Jan 01 2024 at 00:00:00 EET\r\n";
}

// RPL_MYINFO (004)
// "<client> <servername> <version> <available user modes> <available channel modes>"
void	build_RPL_MYINFO(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "004", client.nick);
	buffer += t_IRC_Server::name;
	buffer += ' ';
	buffer += t_IRC_Server::version;
	buffer += " o itkl\r\n";
}

// RPL_ISUPPORT (005)
void	build_RPL_ISUPPORT(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "005", client.nick);
	buffer += "CHANTYPES=#,& CHANLIMIT=#&: CHANMODES=beI,k,l,psit";
	buffer += std::to_string(MAX_CHANNELS_PER_CLIENT);
	buffer += " PREFIX=(o)@ NETWORK=Hive CASEMAPPING=ascii "
		":are supported by this server\r\n";
}

// RPL_CHANNELMODEIS (324)
// "<client> <channel> <mode> <mode params>"
void	build_RPL_CHANNELMODEIS(t_IRC_Client &client, const t_IRC_Channel &channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "324", client.nick);
	buffer += channel.name;
	buffer += ' ';
	append_channel_modes(buffer, channel);
	buffer += "\r\n";
}

// ERR_NOSUCHNICK (401)
// "<client> <nick> :No such nick/channel"
void	build_ERR_NOSUCHNICK(t_IRC_Client &client, std::string_view nick)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "401", client.nick);
	buffer += nick;
	buffer += " :No such nick/channel\r\n";
}

// ERR_NOSUCHCHANNEL (403)
// "<client> <channel name> :No such channel"
void	build_ERR_NOSUCHCHANNEL(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "403", client.nick);
	buffer += channel;
	buffer += " :No such channel\r\n";
}

// ERR_CANNOTSENDTOCHAN (404)
// "<client> <channel name> :Cannot send to channel"
void	build_ERR_CANNOTSENDTOCHAN(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "404", client.nick);
	buffer += channel;
	buffer += " :Cannot send to channel\r\n";
}

// ERR_TOOMANYCHANNELS (405)
// "<client> <channel name> :You have joined too many channels"
void	build_ERR_TOOMANYCHANNELS(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "405", client.nick);
	buffer += channel;
	buffer += " :You have joined too many channels\r\n";
}

// ERR_NORECIPIENT (411)
// "<client> :No recipient given (PRIVMSG/NOTICE)"
void	build_ERR_NORECIPIENT(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "411", client.nick);
	buffer += ":No recipient given (PRIVMSG/NOTICE)\r\n";
}

// ERR_NOTEXTTOSEND (412)
// "<client> :No text to send"
void	build_ERR_NOTEXTTOSEND(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "412", client.nick);
	buffer += ":No text to send\r\n";
}

// ERR_NOMOTD (422)
// "<client> :MOTD File is missing"
void	build_ERR_NOMOTD(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "422", client.nick);
	buffer += ":MOTD File is missing\r\n";
}

// RPL_MOTDSTART (375)
// "<client> :- <server> Message of the day - "
void	build_RPL_MOTDSTART(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "375", client.nick);
	buffer += ":- ";
	buffer += t_IRC_Server::name;
	buffer += " Message of the day -\r\n";
}

// RPL_MOTD (372)
// "<client> :<line of the motd>"
void	build_RPL_MOTD(t_IRC_Client &client, const std::string_view line)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "372", client.nick);
	buffer += ":- ";
	buffer += line;
	buffer += "\r\n";
}

// RPL_ENDOFMOTD (376)
// "<client> :End of /MOTD command"
void	build_RPL_ENDOFMOTD(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "376", client.nick);
	buffer += ":End of /MOTD command\r\n";
}

// ERR_USERNOTINCHANNEL (441)
// "<client> <channel> <nick> :They aren't on that channel"
void	build_ERR_USERNOTINCHANNEL(t_IRC_Client &client,
            std::string_view channel, std::string_view nick)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "441", client.nick);
	buffer += channel;
	buffer += ' ';
	buffer += nick;
	buffer += " :They aren't on that channel\r\n";
}

// ERR_NOTONCHANNEL (442)
// "<client> <channel name> :You're not on that channel"
void	build_ERR_NOTONCHANNEL(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "442", client.nick);
	buffer += channel;
	buffer += " :You're not on that channel\r\n";
}

// ERR_USERONCHANNEL (443)
// "<client> <nick> <channel> :<nick> is on <channel>"
void	build_ERR_USERONCHANNEL(t_IRC_Client &client,
		std::string_view nick, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "443", client.nick);
	buffer += nick;
	buffer += ' ';
	buffer += channel;
	buffer += " :";
	buffer += nick;
	buffer += " is on ";
	buffer += channel;
	buffer += "\r\n";
}

// ERR_CHANNELISFULL (471)
// "<client> <channel name> :Cannot join channel (+l)"
void	build_ERR_CHANNELISFULL(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "471", client.nick);
	buffer += channel;
	buffer += " :Cannot join channel (+l)\r\n";
}

// ERR_UNKNOWNMODE (472)
// "<client> <char> :is unknown mode char to me"
void	build_ERR_UNKNOWNMODE(t_IRC_Client &client, char mode_char)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "472", client.nick);
	buffer += mode_char;
	buffer += " :is unknown mode char to me\r\n";
}

// ERR_INVITEONLYCHAN (473)
// "<client> <channel name> :Cannot join channel (+i)"
void	build_ERR_INVITEONLYCHAN(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "473", client.nick);
	buffer += channel;
	buffer += " :Cannot join channel (+i)\r\n";
}

// ERR_BADCHANNELKEY (475)
// "<client> <channel name> :Cannot join channel (+k)"
void	build_ERR_BADCHANNELKEY(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "475", client.nick);
	buffer += channel;
	buffer += " :Cannot join channel (+k)\r\n";
}

// ERR_BADCHANMASK (476)
// "<client> <channel> :Bad Channel Mask"
void	build_ERR_BADCHANMASK(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "476", client.nick);
	buffer += channel;
	buffer += " :Bad Channel Mask\r\n";
}

// ERR_CHANOPRIVSNEEDED (482)
// "<client> <channel name> :You're not channel operator"
void	build_ERR_CHANOPRIVSNEEDED(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "482", client.nick);
	buffer += channel;
	buffer += " :You're not channel operator\r\n";
}

// RPL_NAMES (353)
// "<client> = <channel> :<nick list>"
void	build_RPL_NAMREPLY(t_IRC_Client &client, std::string_view channel, std::string_view line)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "353", client.nick);
	buffer += "= ";
	buffer += channel;
	buffer += " :";
	buffer += line;
	buffer += "\r\n";
}

// RPL_ENDOFNAMES (366)
// "<client> <channel> :End of /NAMES list`, not `<client> :End of /NAMES list`"
void	build_RPL_ENDOFNAMES(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "366", client.nick);
	buffer += channel;
	buffer += " :End of /NAMES list\r\n";
}

// RPL_LIST (322)
// "<client> <channel> <number of users> :<topic>"
void	build_RPL_LIST(t_IRC_Client &client, const t_IRC_Channel &channel)
{
	std::string		&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "322", client.nick);
	buffer += channel.name;
	buffer += ' ';
	buffer += std::to_string(channel.members.size());
	buffer += " :";
	buffer += channel.topic;
	buffer += "\r\n";
}

// RPL_LISTEND (323)
// "<client> :End of /LIST"
void	build_RPL_LISTEND(t_IRC_Client &client)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "323", client.nick);
	buffer += ":End of /LIST\r\n";
}

// RPL_INVITING (341)
// "<client> <target> <channel>"
void	build_RPL_INVITING(t_IRC_Client &client, std::string_view target_nick, std::string_view channel_name)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "341", client.nick);
	buffer += target_nick;
	buffer += ' ';
	buffer += channel_name;
	buffer += "\r\n";
}

void	build_RPL_NOTOPIC(t_IRC_Client &client, std::string_view channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "331", client.nick);
	buffer += channel;
	buffer += " :No topic is set\r\n";
}

void	build_RPL_TOPIC(t_IRC_Client &client, const t_IRC_Channel &channel)
{
	std::string	&buffer = client.send_message_buffer;

	append_common_reply_prefix(buffer, "332", client.nick);
	buffer += channel.name;
	buffer += " :";
	buffer += channel.topic;
	buffer += "\r\n";
}

void	append_common_reply_prefix(std::string &buffer,
            std::string_view numeric, std::string_view nick)
{
	buffer += ':';
	buffer += t_IRC_Server::name;
	buffer += ' ';
	buffer += numeric;
	buffer += ' ';
	buffer += nick;
	buffer += ' ';
}
