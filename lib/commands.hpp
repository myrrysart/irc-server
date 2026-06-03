#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "irc_fatstruct.hpp"

/* Client registration */
void	client_registration(t_IRC_Client &client, const size_t i, t_IRC_Server &server);
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server);
void	execute_USER_cmd(t_IRC_Client &client, t_IRC_Server &server);

/* Bitmask check helpers */
bool	has_provided_user_and_nick_names(t_bmask mask);
bool	has_provided_correct_password(t_bmask mask);
bool	is_invalid_password_request(const t_bmask state);

/* Invalid command */
void	invalid_command_detected(const t_IRC_Client &client);

// FIXME: Move these away to another header?
/* Numeric Replies */
void	send_ERR_NEEDMOREPARAMS(const t_IRC_Client &client); // 461
void	send_ERR_ALREADYREGISTERED(const t_IRC_Client &client); // 462
void	send_ERR_PASSWDMISMATCH(const t_IRC_Client &client); // 464

#endif
