#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "irc_fatstruct.hpp"

# include <unordered_map>
# include <string_view>

/* Command handling dispatch */
void	dispatch_client_command(t_IRC_Client &client, t_IRC_Server &server);

/* Client registration */
void	client_registration(t_IRC_Client &client, const size_t i, t_IRC_Server &server);
void	execute_PASS_cmd(t_IRC_Client &client, const t_IRC_Server &server);
void	execute_USER_cmd(t_IRC_Client &client);
void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server);

/* IRC Commands */
void	execute_QUIT_cmd(t_IRC_Client &client); // WARN: work in progress

/* Bitmask check helpers */
bool	has_provided_user_and_nick_names(t_bmask mask);
bool	has_provided_password_first_and_it_is_correct(const t_bmask state);
bool	is_or_was_password_provided_first(const t_bmask state);

/* Utils */
bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
            const int fd, const std::string_view new_nick);
bool	is_nickname_valid(const std::string_view nickname);

#endif
