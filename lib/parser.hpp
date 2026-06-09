#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <string_view>

/* Forward declarations */
struct	s_IRC_Server;
typedef s_IRC_Server t_IRC_Server;

struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Received bytes handling */
void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
            const ssize_t received);
void	prepare_and_parse_message(const size_t pos, std::string &buf,
            t_IRC_Client &client);
void	check_for_too_long_message(std::string &buf, t_IRC_Client &client);

/* Parsing & dispatch */
void	tokenize_message(t_IRC_Client &client, const std::string_view &msg);
void	dispatch_client_command(t_IRC_Client &client, t_IRC_Server &server);

/* Parsing utils */
int		init_password(const char *src, std::string_view &dest);
ssize_t	strlen_printable_no_spaces(const char *str);
char	to_uppercase(char c);
bool	are_equal_strs_case_insensitive(const char *str1, const size_t len1,
            const char *str2, const size_t len2);

/* Error logging */
void	log_error(const char *error, const char *filename, int line_number,
            bool is_exception);

#endif
