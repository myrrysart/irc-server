#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "irc_fatstruct.hpp"

void	client_registration(t_IRC_Client &client, size_t i);
void	invalid_command_detected(const t_IRC_Client &client);

#endif
