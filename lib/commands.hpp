#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "irc_fatstruct.hpp"

# include <unordered_map>
# include <string>
# include <string_view>

/* Command handling dispatch */
void	dispatch_client_command(t_IRC_Client &client, t_IRC_Server &server);

/* Client registration */
void	client_registration(t_IRC_Client &client, size_t i, t_IRC_Server &server);
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server);
void	execute_USER_cmd(t_IRC_Client &client);
void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server);

/* IRC Commands */
void	execute_QUIT_cmd(t_IRC_Client &quitter, t_IRC_Server &server);
void	execute_PING_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_PONG_cmd(t_IRC_Client &client, t_IRC_Server &server);

/* Bitmask check helpers */
bool	has_provided_user_and_nick_names(t_bmask mask);
bool	has_provided_password_first_and_it_is_correct(t_bmask state);
bool	is_or_was_password_provided_first(t_bmask state);

/* Utils */
bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
            int fd, std::string_view new_nick);
bool	is_nickname_valid(std::string_view nickname);
void	store_new_nickname(t_IRC_Client &client, std::string_view new_nick);
void	prepare_to_store_new_nick_and_alert_clients(t_IRC_Client &client,
	std::string_view new_nick);
void	build_NICK_message(std::string &nick_msg, t_IRC_Client &client,
	const std::string &old_nick);
void	append_client_quit_msg(std::string &buffer, const t_IRC_Client &quitter);
void	append_nick_user_host(std::string &buffer, const t_IRC_Client &client);

/* Errors */
void	append_common_error_prefix(std::string &output_buffer,
            const char *server_name, const char *hostname);
void	append_error_msg_quit(t_IRC_Client &quitter, const char *server_name);
void	queue_registration_error(std::string &output_buffer,
            const char *server_name, const char *hostname);

#endif
