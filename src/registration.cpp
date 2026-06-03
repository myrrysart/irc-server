
#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"

#include <iostream>
#include <string_view>
#include <new> // for std::bad_alloc

// TODO: Add the time checks!
// Perhaps add some timer machine to t_IRC_Client (per client);
// It would then be turned on as soon as a new client connection is validated
// (in the server loop);
// And it will only be turned off once the client is registered successfully,
// and the REGISTERED flag is set for the client's state - or if registration
// fails entirely and then the client will be disconnected anyways?

// TODO: IF THE USER IS IDLE VERY LONG TIME, KICK THEM OUT!!!
// BUT ONLY DO THAT DURING THEIR REGISTRATION PHASE:
// Do not kick them out for being inactive after they have registered!

// NOTE: From Modern IRC docs: "If the server is waiting to complete a lookup of
// client information (such as hostname or ident for a username), there may be an
// arbitrary wait at some point during registration. Servers SHOULD set a
// reasonable timeout for these lookups.
// Additionally, some servers also send a PING and require a matching PONG from
// the client before continuing. This exchange may happen immediately on connection
// and at any time during connection registration, so clients MUST
// respond correctly to it."
// "Upon successful completion of the registration process, the server MUST send,
// in this order:
// RPL_WELCOME (001),
// RPL_YOURHOST (002),
// RPL_CREATED (003),
// RPL_MYINFO (004),
// at least one RPL_ISUPPORT (005) numeric to the client.
// The server MAY then send other numerics and messages.
// The server SHOULD then respond as though the client sent the LUSERS command
// and return the appropriate numerics.
// The server MUST then respond as though the client sent it the MOTD command,
// i.e. it must send either the successful Message of the Day numerics or the
// ERR_NOMOTD (422) numeric.
// If the user has client modes set on them automatically upon joining the network,
// the server SHOULD send the client the RPL_UMODEIS (221) reply or a MODE message
// with the client as target, preferably the former.
// The first parameter of the RPL_WELCOME (001) message is the nickname assigned
// by the network to the client. Since it may differ from the nickname the client
// requested with the NICK command (due to, e.g. length limits or policy
// restrictions on nicknames), the client SHOULD use this parameter to determine
// its actual nickname at the time of connection. Subsequent nickname changes,
// client-initiated or not, will be communicated by the server sending a NICK message.

void	client_registration(t_IRC_Client &client, const size_t i, t_IRC_Server &server)
{
	switch (i)
	{
		default: {
			if (i < t_parser::n_valid_cmds)
				std::cout << "Registration incomplete\n"; // FIXME: send instead of printing. Is there a numeric for this? This was just copied from Irssi's output.
			else
				invalid_command_detected(client);
		} break;
		case 0: execute_PASS_cmd(client, server); break;
		// case 1: execute_NICK_cmd(); break;
		case 2: execute_USER_cmd(client, server); break;
	}

	if (has_provided_user_and_nick_names(client.state))
	{
		if (has_provided_correct_password(client.state))
		{
			// TODO: client registered successfully.
			// go on to do all of the steps which server has to do when
			// client registers successfully...

			client.state |= t_IRC_Client::REGISTERED;


			// WARN: debugging only
			std::cout
				<< "Registration: SUCCESS!\n"
				<< "\n\nBITMASK of client is = " << client.state << "\n\n";

		}
		else
		{
			// WARN: It seems like the "dd horse" protocol treats the same way:
			// 	- NICK / USER provided before password, and
			// 	- wrong password
			// 	It executes the following steps for both:
			// 	1. sending ERR_PASSWDMISMATCH 464 ("Password Incorrect" message for example)
			// 	2. sending another Error: "ERROR :Closing Link: localhost (Bad Password)"
			// 	3. Disconnecting the client.

			// TODO: missing elements here.
			// send password mismatch 464
			send_ERR_PASSWDMISMATCH(client);

			// send ERROR ?

			// set DISCONNECT FLAG?
			client.state |= t_IRC_Client::ERROR;


			// WARN: debugging only
			std::cout << "\n\nBITMASK of client is = " << client.state << "\n\n";

		}
	}
}

bool	has_provided_user_and_nick_names(const t_bmask state)
{
	t_bmask	temp = t_IRC_Client::NICK | t_IRC_Client::USERNAME;

	if ((state & temp) == temp)
		return true;
	return false;
}

bool	has_provided_correct_password(const t_bmask state)
{
	t_bmask	temp = t_IRC_Client::PSWD_FIRST | t_IRC_Client::PSWD_CORRECT;

	if ((state & temp) == temp)
		return true;
	return false;

}

// WARN: Can this function safely ignore any parameters after 1st param (client.parser.params[0])?
// TODO: Missing: timeout handling.
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server)
{
	// check if already registered
	if ((client.state & t_IRC_Client::REGISTERED) == t_IRC_Client::REGISTERED)
	{
		// TODO: send ERR_ALREADYREGISTERED (462)
		send_ERR_ALREADYREGISTERED(client);
		return;
	}

	// check that a password was provided in current message
	if (!client.parser.n_params)
	{
		// TODO: send ERR_NEEDMOREPARAMS (461)
		send_ERR_NEEDMOREPARAMS(client);
		return;
	}

	if (is_invalid_password_request(client.state))
		return;

	// set client's 'password provided first' flag (do NOT unset if already set!)
	client.state |= t_IRC_Client::PSWD_FIRST;

	// check if the password is the right one or not, and adjust PSWD_CORRECT flag
	if (client.parser.params[0] == server.password)
		client.state |= t_IRC_Client::PSWD_CORRECT; // set PSWD_CORRECT flag
	else
		client.state &= ~t_IRC_Client::PSWD_CORRECT; // unset PSWD_CORRECT flag (or make sure it remains unset, do not flip)
}

/* Since this IRC server requires a password in order for a client to register,
* IRC protocol makes it clear that the client has to provide the PASS command
* first, before sending either NICK or USER. This check has to be done at the
* top of execute_PASS_cmd() - right after the check for whether the client is
* already registered or not. This function takes into consideration all the
* possible combinations of the three flags concerned: PSWD_FIRST, NICK, USERNAME,
* and only returns true in the correct cases (if the function is called at the
* right time.) */
bool	is_invalid_password_request(const t_bmask state)
{
	// check if PSWD_FIRST is unset
	if ((state & t_IRC_Client::PSWD_FIRST) != t_IRC_Client::PSWD_FIRST)
	{
		// check if at least one of either NICK or USERNAME are set
		if  (((state & t_IRC_Client::NICK) == t_IRC_Client::NICK)
			|| ((state & t_IRC_Client::USERNAME) == t_IRC_Client::USERNAME))
			return true;
	}
	return true;
}

// TODO: work in progress: function not ready.
void	execute_USER_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// check if already registered
	if ((client.state & t_IRC_Client::REGISTERED) == t_IRC_Client::REGISTERED)
	{
		// TODO: send ERR_ALREADYREGISTERED (462).
		send_ERR_ALREADYREGISTERED(client);
		return;
	}

	// check if a username was actually provided in current message
	// WARN: should I change this next boolean to:
	// if (client.parser.n_params != 4 || client.parser.params[3] == '') ????
	if (!client.parser.n_params)
	{
		// TODO: send ERR_NEEDMOREPARAMS (461)
		send_ERR_NEEDMOREPARAMS(client);
		return;
	}

	// set USERNAME flag (should be done whether password has already been provided or not)
	client.state |= t_IRC_Client::USERNAME;

	// check that a password was provided before this command
	if ((client.state & t_IRC_Client::PSWD_FIRST) == t_IRC_Client::PSWD_FIRST)
	{
		std::string_view	&param = client.parser.params[0];

		// TODO:
		// continue registration here
		try { // std::string often dynamically allocates new resources!
			client.username = "~";
			client.username.append(param.substr(0, t_IRC_Client::username_len));
		} catch (const std::bad_alloc &e) {

			std::cerr << "Exception caught: " << e.what() << std::endl;
			// set fatal error flags!
			client.state |= t_IRC_Client::ERROR;
			// server.state |= SERVER_ERROR; // WARN: add the right flag after PR is merged.
			(void)server; // WARN: temporary since server flag does not exist.
			return;
		}


		// check if the realname is available // WARN: Am I sure about this?
		// TODO:





	}
	// NOTE:
	// Server requires passowrd, which means that the user HAS to provide
	// it FIRST. Following the example on:
	// https://dd.ircdocs.horse/refs/commands/passbut
	// This requires silently accepting any NICK / USER - and even PASS
	// future combinations, including the present USER command - but the
	// client will anyways end up getting disconnected, because current
	// code structure would not allow setting the PSWD_CORRECT flag - and
	// server will return the following error, once NICK has also been provided:
	// "ERROR :Closing Link: localhost (Bad Password)"
	// NOTE: I suppose it is unnecessary to store the username, if the 'password
	// given first' flag is not on. Same goes for NICK.
}


// TODO: NICK: cap nicknames at 30 characters, and trim any characters beyond that
// without saying anything. Example from the old Horse docs:
// 'dan-is-my-name-dont-wear-it-out-at-all' became: 'dan-is-my-name-dont-wear-it-ou'
// NOTE: In this example on Horse, the returned welcome 001 message does not
// end with the nickname at all, unlike other, shorter examples?!

// TODO: Consider cases of commands that are received when they are not valid
// anymore - for example, a nickname has to be set before


// TODO: Follow flow of USER command, regarding the PSWD_FIRST flag.
// TODO: write exec_NICK_cmd()
// void	execute_NICK_cmd(const t_IRC_Client &client)




// FIXME: move all numeric communication functions to dedicated file/s


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


// WARN: Should we support CAP - capability negotiation? Probably unnecessary.

// FIXME: Should we accept multiple connections from the same client (perhaps there
// is a way for them to connect once, and then somehow change their nickname/name to a
// valid one, and then they could be still validated again? There is a way to check
// the IP address via the socket address info struct of the client.... but is it necessary?)

// NOTE: "Clients MUST NOT include a source when sending a message. Servers MAY
// include a source on any message, and MAY leave a source off of any message.
// Clients MUST be able to process any given message the same way whether it
// contains a source or does not contain one."

// TODO: "When sending messages, ensure that a pair of \r\n characters follows
// every single message your software sends out"
