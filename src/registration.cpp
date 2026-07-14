
#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"

#include <unordered_map>
#include <chrono>
#include <string>

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
