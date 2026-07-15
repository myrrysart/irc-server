#include "../lib/irc_fatstruct.hpp"
#include "../lib/numerics.hpp"

/* Server requires passowrd, which means that the user HAS to provide it FIRST.
* Following the example on: https://dd.ircdocs.horse/refs/commands/passbut,
* this requires silently accepting any NICK / USER - and even PASS future
* combinations, including the present USER command - but the client will anyways
* end up getting disconnected, because current code structure would not allow
* setting the PSWD_CORRECT flag - and server will return the following error,
* once NICK has also been provided:
* "ERROR :Closing Link: localhost (Bad Password)"
*
* It is unnecessary to store the username, if the 'password given first'
* flag is not on, since it will be ignored. Same goes for NICK. */
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

	std::string_view	*params = client.parser.params;
	client.username = "~"; // a prefix indicating that 'username' is set by the user.
	// silently trim any characters after USERLEN ('~' counts)
	client.username.append(params[0].substr(0, t_IRC_Client::USERLEN - 1));
	client.realname = params[3];
	/* as for parameters [1] & [2]: they are usually sent from the client
	* as '0' and '*', respectively - but they do not really concern anything
	* in the current scope, and the server can silently ignore these. */
}
