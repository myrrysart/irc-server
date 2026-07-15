
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
		default: build_ERR_NOTREGISTERED(client);   break;
		case 0:  execute_CAP_cmd(client);           break;
		case 1:  execute_PASS_cmd(client, server);  break;
		case 2:  execute_NICK_cmd(client, server);  break;
		case 3:  execute_USER_cmd(client);          break;
		case 4:  execute_QUIT_cmd(client, server);  break;
	}


	if (has_provided_both_user_and_nick(client.state))
	{
		if (has_provided_a_password(client.state))
		{
			if (has_provided_credents_in_right_order_and_pass_is_correct(client.state))
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
				/* client has failed the registration process */
				client.state |= t_IRC_Client::DISCONNECT;
				build_ERR_PASSWDMISMATCH(client);
				queue_registration_error(client.send_message_buffer, server.name,
					client.hostname);
			}
		}
		else
		{
			/* Wrong order!
			*  client has not provided password yet, but they did provide both
			*  NICK & USER. Grant them possiblity to still provide the password;
			*  but registration will end up failing anyways */
			client.state |= t_IRC_Client::WRONG_ORDER;
		}
	}
}

bool	is_flag_set(t_bmask state, t_bmask mask)
{
	if ((state & mask) == mask)
		return true;
	return false;
}

bool	has_provided_a_password(t_bmask state)
{
	if (is_flag_set(state, t_IRC_Client::PSWD_GIVEN))
		return true;
	return false;
}

bool	has_provided_credents_in_right_order_and_pass_is_correct(t_bmask state)
{
	if (is_flag_set(state, t_IRC_Client::WRONG_ORDER))
		return false;

	if (is_flag_set(state, t_IRC_Client::PSWD_CORRECT))
		return true;

	return false;
}

bool	has_provided_both_user_and_nick(t_bmask state)
{

	if (is_flag_set(state, (t_IRC_Client::NICK | t_IRC_Client::USERNAME)))
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
