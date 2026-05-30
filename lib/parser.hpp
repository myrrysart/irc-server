#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>

/* forward declarations */
struct s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Received bytes handling */
void	handle_message_to_discard(t_IRC_Client &client, const char *buf, const ssize_t received);
void	prepare_and_parse_message(const size_t pos, std::string &buf, t_IRC_Client &client);
void	check_for_too_long_message(std::string &buf, t_IRC_Client &client);

/* Parsing & dispatch */
void	tokenize_message(t_IRC_Client &client, const std::string_view &msg);
void	dispatch_client_command(const t_IRC_Client &client);

/* Parsing utils */
char	to_uppercase(char c);





// WARN: Only for debugging purposes: remember to delete
void	display_tokens(const t_IRC_Client &client);

#endif
