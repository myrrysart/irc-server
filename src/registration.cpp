
#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

#include <string_view>
#include <unordered_map>
#include <string> // for std::string's append()
#include <algorithm> // for std::min()
#include <cctype> // for std::isdigit()

// TODO: Add the time checks!
// Perhaps add some timer machine to t_IRC_Client (per client);
// It would then be turned on as soon as a new client connection is validated
// (in the server loop);
// And it will only be turned off once the client is registered successfully,
// and the REGISTERED flag is set for the client's state - or if registration
// fails entirely and then the client will be disconnected anyways?

// TODO: IF THE USER IS IDLE VERY LONG TIME, kick them out.
// BUT ONLY DO THAT DURING THEIR REGISTRATION PHASE:
// Do not kick them out for being inactive after they have registered.

// NOTE: From Modern IRC docs: "If the server is waiting to complete a lookup of
// client information (such as hostname or ident for a username), there may be an
// arbitrary wait at some point during registration. Servers SHOULD set a
// reasonable timeout for these lookups.
// Additionally, some servers also send a PING and require a matching PONG from
// the client before continuing. This exchange may happen immediately on connection
// and at any time during connection registration, so clients MUST respond
// correctly to it."
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
		default: build_ERR_NOTREGISTERED(client); break;
		case 0: execute_PASS_cmd(client, server); break;
		case 1: execute_NICK_cmd(client, server); break;
		case 2: execute_USER_cmd(client);         break;
		case 3: execute_QUIT_cmd(client, server); break;
	}

	if (has_provided_user_and_nick_names(client.state))
	{
		if (has_provided_password_first_and_it_is_correct(client.state))
		{
			// TODO: client registered successfully.
			// go on to do all of the steps which server has to do when
			// client registers successfully...

			client.state |= t_IRC_Client::REGISTERED;

			build_RPL_WELCOME(client);
			build_RPL_YOURHOST(client);
			// TODO:
			// missing steps here: more numeric replies are needed.

		}
		else
		{
			// WARN: It seems like the "dd horse" protocol treats the same way:
			// 	• Wrong password
			// 	• NICK / USER provided before password
			// 	It executes the following steps for both situtations:
			// 	1. Send ERR_PASSWDMISMATCH 464 ("Password Incorrect" message for example)
			// 	2. Send another Error: "ERROR :Closing Link: localhost (Bad Password)"
			// 	3. Disconnect the client.

			// TODO: missing elements here.
			// send password mismatch 464
			build_ERR_PASSWDMISMATCH(client);

			// TODO:
			// send ERROR ?


			// set disconnect flag
			client.state |= t_IRC_Client::DISCONNECT;
		}
	}
}

bool	is_flag_set(const t_bmask state, const unsigned int mask)
{
	if ((state & mask) == mask)
		return true;
	return false;
}

bool	has_provided_user_and_nick_names(const t_bmask state)
{

	if (is_flag_set(state, (t_IRC_Client::NICK | t_IRC_Client::USERNAME)))
		return true;
	return false;
}

bool	has_provided_password_first_and_it_is_correct(const t_bmask state)
{
	if (is_flag_set(state, (t_IRC_Client::PSWD_FIRST | t_IRC_Client::PSWD_CORRECT)))
		return true;
	return false;
}

// TODO: Missing: timeout handling.
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server)
{
	// check if already registered
	if (is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		build_ERR_ALREADYREGISTERED(client);
		return;
	}

	// check that a password was provided in current message
	if (!client.parser.n_params)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	if (is_or_was_password_provided_first(client.state))
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
* first, before sending either NICK or USER. This check has to be done as soon
* as a valid PASS command (i.e. a command which includes a password argument)
* has been received by the server, from a non-registered client. However, the
* protocol allows the client to send the PASS command multitple times during the
* registration process, and only checks whether the password is right when both
* NICK and USERNAME have been provided, following an initial valid PASS command.
* This function takes into consideration all the possible combinations of the
* three flags concerned: PSWD_FIRST, NICK and USERNAME. It returns true only in
* the appropriate cases */
bool	is_or_was_password_provided_first(const t_bmask state)
{
	// check if PSWD_FIRST is unset
	if (!is_flag_set(state, t_IRC_Client::PSWD_FIRST))
	{
		// check if at least one of either NICK or USERNAME are set
		if (is_flag_set(state, t_IRC_Client::NICK)
			|| is_flag_set(state, t_IRC_Client::USERNAME))
			return true;
	}
	return false;
}

void	execute_USER_cmd(t_IRC_Client &client)
{
	// check if already registered
	if (is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		build_ERR_ALREADYREGISTERED(client);
		return;
	}

	/* check that enough parameters are provided; if they are, check that the
	* 1st parameter ('username') is not empty. This strictly follows protocol. */
	if (client.parser.n_params < 4 || client.parser.params[0].empty())
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}

	// set USERNAME flag (should be done whether password has already been provided or not)
	client.state |= t_IRC_Client::USERNAME;

	// check that a password was provided first during client registration
	if (is_flag_set(client.state, t_IRC_Client::PSWD_FIRST))
	{
		std::string_view	*params = client.parser.params;
		client.username = "~"; // a prefix indicating that 'username' is set by the user.
		client.username.append(params[0].substr(0, t_IRC_Client::userlen)); // silently trim any characters after userlen
		client.realname = params[3];
		/* as for parameters [1] & [2]: they are usually sent from the client
		* as '0' and '*', respectively - but they do not really concern anything
		* in the current scope, and the server can silently ignore these. */
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
// NOTE: Maybe this last part is about the size of the send buffer, and that's why
// the nick got entirely truncated there - it already displays it in the message
// beforehand, making the string very long.

void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no nickname / empty nickname provided
	if (!client.parser.n_params || client.parser.params[0].empty())
	{
		build_ERR_NONICKNAMEGIVEN(client);
		return;
	}

	std::string_view	new_nick{client.parser.params[0].data(),
		std::min(client.parser.params[0].size(), t_IRC_Client::max_nicklen)};
	size_t	new_nicklen = new_nick.size();

	// validation: check that requested nickname contains only valid characters
	// NOTE: It is important that this step is done here, and not later on.
	// Upon setup of a new client, the default nickname is set to '*', which is
	// an invalid placeholder - on purpose.
	// A client requesting the nickname '*' should be refused, and such a nickname
	// should not be compared against any other default clients' nickname, when
	// this function checks whether it is already taken...
	if (!is_nickname_valid(new_nick))
	{
		build_ERR_ERRONEOUSNICKNAME(client, new_nick);
		return;
	}

	// silently ignore request for an already identical nickname (same client)
	if (is_flag_set(client.state, t_IRC_Client::NICK)
		&& are_equal_strs_case_insensitive(new_nick.data(), new_nicklen,
			client.nick.data(), client.nick.size()))
		return;


	// handle an already taken nickname
	if (is_nick_already_in_use(server.clients, client.fd, new_nick))
	{
		build_ERR_NICKNAMEINUSE(client, new_nick);
		return;
	}

	// nickname request is valid: update client's state
	client.state |= t_IRC_Client::NICK;

	// Only store nickname if client is:
	//  • already registered
	//  • unregistered, but has provided the password first
	if (is_flag_set(client.state, t_IRC_Client::REGISTERED)
		|| is_flag_set(client.state, t_IRC_Client::PSWD_FIRST))
	{

		// Store new nickname (has to be a deep copy)
		for (size_t	i = 0; i < new_nicklen; ++i)
			client.nick_buf[i] = new_nick[i];
		client.nick = std::string_view{client.nick_buf, new_nicklen};

		// TODO: Build message/s to be sent message to concerned clients/channels
		// regarding the nick name change of current client? Look for appropriate
		// numeric reply and implement.

	}
	// 'else', we return, silently ignoring the NICK change: no need to inform
	// anyone about it since client is not yet connected and no one else is aware
	// of it, and client will end up being disconnected with ERR_PASSWDMISMATCH
}

// WARN: Review this function when CHANTYPES are chosen for channel handling!
// Those characters have to be avoided here!
bool	is_nickname_valid(const std::string_view nickname)
{
	size_t	len = nickname.size();

	// refuse leading: digit, '#', ':' and "&#" // WARN: review this when CHANTYPES are known!
	if (len)
	{
		char	c = nickname[0];
		if (std::isdigit(static_cast<unsigned char>(c)) || c == '#' || c == ':')
			return false;
	}
	if (len > 1 && nickname.compare(0, 2, "&#") == 0)
		return false;

	if (nickname.find_first_not_of(t_IRC_Client::nick_whitelist) != std::string_view::npos)
		return false;
	return true;
}

// uses 'ascii' CASEMAPPING, meaning case-insensitive checks: "tommy" == "ToMMY"
bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
            const int fd, const std::string_view new_nick)
{
	size_t	new_nicklen = new_nick.size();

	// iterator->first is the fd key, iterator->second is t_IRC_Client value
	for (std::unordered_map<int, t_IRC_Client>::const_iterator iterator = clients.begin();
		iterator != clients.end();
		++iterator)
	{
		// avoid comparing newly requested nickname with requesting client's current nickname
		if (iterator->first == fd)
			continue;

		if (are_equal_strs_case_insensitive(new_nick.data(), new_nicklen,
				iterator->second.nick.data(), iterator->second.nick.size()))
			return true;
	}
	return false;
}

// WARN: Should we support CAP - capability negotiation? Probably unnecessary.

// FIXME: Should we accept multiple connections from the same client (perhaps there
// is a way for them to connect once, and then somehow change their nickname/name to a
// valid one, and then they could be still validated again? There is a way to check
// the IP address via the socket address info struct of the client.... but is it necessary?)
