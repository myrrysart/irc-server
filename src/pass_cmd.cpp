#include "../lib/irc_fatstruct.hpp"
#include "../lib/numerics.hpp"
#include "../lib/commands.hpp"

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
