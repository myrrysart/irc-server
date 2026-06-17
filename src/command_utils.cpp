#include "../lib/irc_fatstruct.hpp"
#include "../lib/commands.hpp"

#include <string>

// this function may throw
void	append_client_quit_msg(std::string &buffer, const t_IRC_Client &quitter)
{
	append_nick_user_host(buffer, quitter);
	buffer += " QUIT :Quit: ";
	if (quitter.parser.n_params)
		buffer += quitter.parser.params[0];
	buffer += "\r\n";
}

// this function may throw
void	append_nick_user_host(std::string &buffer, const t_IRC_Client &client)
{
	buffer += client.nick;
	buffer += '!';
	buffer += client.username;
	buffer += '@';
	buffer += client.hostname;
}
