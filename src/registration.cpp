
#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include "../lib/channel.hpp"

#include <unordered_map>
#include <string> // for std::string's append()
#include <string_view>
#include <cctype> // for std::isdigit()

static void	prepare_to_store_new_nick_and_alert_clients(t_IRC_Client &client,
	            std::string_view new_nick);

void	client_registration(t_IRC_Client &client, size_t i, t_IRC_Server &server)
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
			/* client registered successfully */
			client.state |= t_IRC_Client::REGISTERED;

			build_RPL_WELCOME(client);
			build_RPL_YOURHOST(client);
			build_RPL_CREATED(client);
			build_RPL_MYINFO(client);
			build_RPL_ISUPPORT(client);

			build_RPL_MOTDSTART(client);
			build_RPL_MOTD(client, "Welcome to our humble server!");
			build_RPL_ENDOFMOTD(client);

		}
		else
		{
			/* The "dd horse" protocol treats all of the following registration
			* scnarios as failed attempts. It requires sending the numeric reply
			* ERR_PASSWDMISMATCH and an error message before disconnecting:
			* • Wrong password
			* • NICK / USER provided before password
			* • both NICK and USER provided but no password */
			build_ERR_PASSWDMISMATCH(client);
			queue_registration_error(client.send_message_buffer, server.name,
				client.hostname);
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

bool	has_provided_user_and_nick_names(t_bmask state)
{

	if (is_flag_set(state, (t_IRC_Client::NICK | t_IRC_Client::USERNAME)))
		return true;
	return false;
}

bool	has_provided_password_first_and_it_is_correct(t_bmask state)
{
	if (is_flag_set(state, (t_IRC_Client::PSWD_FIRST | t_IRC_Client::PSWD_CORRECT)))
		return true;
	return false;
}

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

	// check if the password is the right one or not;
	// set / unset the PSWD_CORRECT flag accordingly
	if (client.parser.params[0] == server.password)
		client.state |= t_IRC_Client::PSWD_CORRECT;
	else
		client.state &= ~t_IRC_Client::PSWD_CORRECT;
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
bool	is_or_was_password_provided_first(t_bmask state)
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
		// silently trim any characters after userlen ('~' counts)
		client.username.append(params[0].substr(0, t_IRC_Client::userlen - 1));
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
	// It is unnecessary to store the username, if the 'password given first'
	// flag is not on, since it will be ignored. Same goes for NICK.
}

static void	store_new_nickname(t_IRC_Client &client, std::string_view new_nick)
{
	for (size_t	i = 0; i < new_nick.size(); ++i)
		client.nick_buf[i] = new_nick[i];
	client.nick = std::string_view{client.nick_buf, new_nick.size()};
}

void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no nickname / empty nickname provided
	if (!client.parser.n_params || client.parser.params[0].empty())
	{
		build_ERR_NONICKNAMEGIVEN(client);
		return;
	}

	std::string_view	new_nick{client.parser.params[0]};
	trim_nickname_if_longer_than_max_nicklen(new_nick);

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

	// Silently ignore request for an already identical nickname (same client).
	// However, if the equality is only 'case-insensitive', change the nickname,
	// alert the client and fellow channel members
	if (is_flag_set(client.state, t_IRC_Client::NICK)
			&& are_equal_strs_case_insensitive(new_nick, client.nick))
	{
		if (!are_equal_strs_case_sensitive(new_nick, client.nick))
			prepare_to_store_new_nick_and_alert_clients(client, new_nick);
		return;
	}

	// handle an already taken nickname
	if (is_nick_already_in_use(server.clients, client.fd, new_nick))
	{
		build_ERR_NICKNAMEINUSE(client, new_nick);
		return;
	}

	prepare_to_store_new_nick_and_alert_clients(client, new_nick);
}

static void	prepare_to_store_new_nick_and_alert_clients(t_IRC_Client &client,
	            std::string_view new_nick)
{
	// nickname request is valid: update client's state
	client.state |= t_IRC_Client::NICK;

	// Only store new nickname if client is:
	//  • already registered
	//  • unregistered, but has provided the password first
	if (is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		// temporarily store previous nick in order to integrate it to the reply
		std::string	old_nick{client.nick};

		// Store new nickname (has to be a deep copy)
		store_new_nickname(client, new_nick);

		// Broadcast NICK reply to the requesting client and to all fellow
		// channelers (but not more than once per client)
		broadcast_nick_change(client, old_nick);

	}
	else if (is_flag_set(client.state, t_IRC_Client::PSWD_FIRST))
	{
		// Client registration process seems to be going well:
		// Store new nickname (has to be a deep copy)
		store_new_nickname(client, new_nick);
	}
	/* 'else': we return, silently ignoring the NICK change. No need to inform
	* anyone about it since the client is not yet connected and no one else is
	* aware of its connection attempt - and it will be disconnected soon enough,
	* since their registration process has failed */
}

static void	build_NICK_message(std::string &nick_msg, t_IRC_Client &client,
	            const std::string &old_nick)
{
	const std::string_view	slice = " NICK ";
	size_t	len = old_nick.size() + client.username.size()
		+ sizeof(client.hostname) + slice.size() + client.nick.size() + 6;
	// 6: ':' + '!' + '@' + ':' + '\r' + '\n'

	nick_msg.reserve(len);

	nick_msg += ':';
	nick_msg += old_nick;
	nick_msg += '!';
	nick_msg += client.username;
	nick_msg += '@';
	nick_msg += client.hostname;
	nick_msg += slice;
	nick_msg += ':';
	nick_msg += client.nick;
	nick_msg += "\r\n";
}

void	broadcast_nick_change(t_IRC_Client &client, const std::string &old_nick)
{
	// build the reply
	std::string	nick_msg;
	build_NICK_message(nick_msg, client, old_nick);

	// queue the reply to requesting client's output buffer
	client.send_message_buffer += nick_msg;

	// broadcast to all fellow channelers - but only once per client, even if
	// they happen to share more than one channel with the client
	broadcast_to_fellow_channelers_once_per_client(client, nick_msg);
}

// WARN: Review this function when CHANTYPES are chosen for channel handling!
// Those characters have to be avoided here!
bool	is_nickname_valid(std::string_view nickname)
{
	size_t	len = nickname.size();

	// refuse leading: digit, '#', ':' or '&'
	if (len)
	{
		char	c = nickname[0];
		if (std::isdigit(static_cast<unsigned char>(c))
				|| c == '#' || c == ':' || c == '&')
			return false;
	}

	if (nickname.find_first_not_of(t_IRC_Client::nick_whitelist) != std::string_view::npos)
		return false;
	return true;
}

// uses 'ascii' CASEMAPPING, meaning case-insensitive checks: "tommy" == "ToMMY"
bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
            int fd, std::string_view new_nick)
{
	// iterator->first is the fd key, iterator->second is t_IRC_Client value
	for (std::unordered_map<int, t_IRC_Client>::const_iterator iterator = clients.begin();
		iterator != clients.end();
		++iterator)
	{
		// avoid comparing newly requested nickname with requesting client's current nickname
		if (iterator->first == fd)
			continue;

		if (are_equal_strs_case_insensitive(new_nick, iterator->second.nick))
			return true;
	}
	return false;
}

void	check_registration_timeouts(t_IRC_Server &server)
{
	for (std::unordered_map<int, t_IRC_Client>::iterator it = server.clients.begin();
		it != server.clients.end(); ++it)
	{
		if (!is_flag_set(it->second.state, t_IRC_Client::REGISTERED)
			&& !is_flag_set(it->second.state, t_IRC_Client::DISCONNECT))
		{
			if (std::chrono::steady_clock::now() - it->second.connection_time
					> std::chrono::seconds(t_IRC_Server::registration_timeout))
			{
				append_common_error_prefix(it->second.send_message_buffer,
					server.name, it->second.hostname);
				it->second.send_message_buffer += " (registration timeout)\r\n";
				it->second.state |= t_IRC_Client::DISCONNECT;
			}
		}
	}
}
