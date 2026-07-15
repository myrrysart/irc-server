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

	// password provided succesfully
	client.state |= t_IRC_Client::PSWD_GIVEN;

	// check if the password is the right one or not;
	// set / unset the PSWD_CORRECT flag accordingly
	if (client.parser.params[0] == server.password)
		client.state |= t_IRC_Client::PSWD_CORRECT;
	else
		client.state &= ~t_IRC_Client::PSWD_CORRECT;
}
