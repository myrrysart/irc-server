#include "../lib/irc_fatstruct.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

#include <string_view>
#include <string>

static void	queue_CAP_reply(t_IRC_Client &client, bool is_REQ);

void	execute_CAP_cmd(t_IRC_Client &client)
{
	if (!client.parser.n_params)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	std::string_view	first_argument = client.parser.params[0];

	if (are_equal_strs_case_insensitive(first_argument, "LS")
		|| are_equal_strs_case_insensitive(first_argument, "LIST"))
	{
		queue_CAP_reply(client, false);
	}
	else if (are_equal_strs_case_insensitive(first_argument, "REQ"))
	{
		if (client.parser.n_params < 2)
		{
			build_ERR_NEEDMOREPARAMS(client); // 461
			return;
		}
		queue_CAP_reply(client, true);
	}
	else if (are_equal_strs_case_insensitive(first_argument, "END"))
	{
		; // explicitly ignore CAP with END argument
	}
	// else: ignore
}

static void	queue_CAP_reply(t_IRC_Client &client, bool is_REQ)
{
	std::string	&send_buf = client.send_message_buffer;

	send_buf += ':';
	send_buf += t_IRC_Server::name;
	send_buf += ' ';
	send_buf += std::string_view{client.parser.verb_in_caps, client.parser.verb.size()};
	send_buf += " * ";
	if (is_REQ)
	{
		send_buf += "NAK :";
		send_buf += client.parser.params[1];
	}
	else
	{
		std::string	uppercase_arg;
		for (size_t i = 0; i < client.parser.params[0].size(); ++i)
			uppercase_arg += to_uppercase(client.parser.params[0][i]);
		send_buf += uppercase_arg;
		send_buf += " :";
	}
	send_buf += "\r\n";
}
