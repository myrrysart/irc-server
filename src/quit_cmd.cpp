#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"
#include "../lib/channel.hpp"

#include <string>
#include <string_view>

/* message string builder for execute_QUIT_cmd() */
void	append_quit_message(std::string &quit_msg, const t_IRC_Client &quitter,
	            bool is_quit_requested)
{
	const std::string_view	middle_part{" QUIT :Quit: "};
	const std::string_view	fatal_error{"Connection closed"};

	// calculate how long the message will be, to allow pre-reservation of
	// capacity, avoiding potential std::string reallocations
	size_t	len = quitter.nick.size() + quitter.username.size()
		+ sizeof(quitter.hostname) + middle_part.size() + 5; // 5: :, !, @, \r\n
	if (is_quit_requested && quitter.parser.n_params > 0)
		len += quitter.parser.params[0].size();
	else
		len += fatal_error.size();

	quit_msg.reserve(len);

	quit_msg += ':';
	append_nick_user_host(quit_msg, quitter);

	if (is_quit_requested)
	{
		// append 'middle_part' followed by the reason provided by 'quitter'.
		// If no reason provided or reason is empty: the client should still
		// receive ":Quit: ", implemented in 'middle_part'
		quit_msg += middle_part;
		if (quitter.parser.n_params > 0)
			quit_msg += quitter.parser.params[0];
	}
	else
	{
		// non-graceful exit:
		// fellow channellers receive slightly different messsage
		quit_msg += middle_part.substr(0, 7);
		quit_msg += fatal_error;
	}
	quit_msg += "\r\n";
}

void    execute_QUIT_cmd(t_IRC_Client &quitter, t_IRC_Server &server)
{
	// According to the Modern IRC protocol: "The server acknowledges" the
	// QUIT command "by replying with an ERROR message"
	append_error_msg_quit(quitter, server.name);

	// trigger disconnection of client without alerting anyone,
	// if client is unregistered or not on any channel
	if (!is_flag_set(quitter.state, quitter.REGISTERED)
		|| quitter.joined_channels.empty())
	{
		quitter.state |= t_IRC_Client::DISCONNECT;
		return;
	}

	// build message to be broadcasted. Each client that shares a channel with
	// the departing client should be notified
	std::string	quit_msg;
	append_quit_message(quit_msg, quitter, true);

	broadcast_to_fellow_channelers_once_per_client(quitter, quit_msg);
	quitter.state |= t_IRC_Client::DISCONNECT;
}
