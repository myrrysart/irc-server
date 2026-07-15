#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "irc_fatstruct.hpp"

# include <string>

/* Command handling dispatch */
void	dispatch_client_command(t_IRC_Client &client, t_IRC_Server &server);

/* Client registration */
void	client_registration(t_IRC_Client &client, size_t i, t_IRC_Server &server);
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server);
void	execute_USER_cmd(t_IRC_Client &client);
void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_CAP_cmd(t_IRC_Client &client);

/* IRC Commands */
void	execute_QUIT_cmd(t_IRC_Client &quitter, t_IRC_Server &server);
void	execute_PING_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_PONG_cmd(t_IRC_Client &client, t_IRC_Server &server);

/* Bitmask check helpers */
bool	has_provided_a_password(t_bmask state);
bool	has_provided_both_user_and_nick(t_bmask state);
bool	has_provided_credents_in_right_order_and_pass_is_correct(t_bmask state);

/* Utils */
void	append_client_quit_msg(std::string &buffer, const t_IRC_Client &quitter);
void	append_nick_user_host(std::string &buffer, const t_IRC_Client &client);

/* Errors */
void	append_common_error_prefix(std::string &output_buffer,
            const char *server_name, const char *hostname);
void	append_error_msg_quit(t_IRC_Client &quitter, const char *server_name);
void	queue_registration_error(std::string &output_buffer,
            const char *server_name, const char *hostname);

#endif
