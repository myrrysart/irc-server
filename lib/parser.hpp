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
int		prepare_and_parse_message(const size_t pos, const std::string &buf,
            t_IRC_Client &client);
int		check_for_too_long_message(std::string &buf, t_IRC_Client &client);

/* Parsing */
void	tokenize_message(t_IRC_Client &client, const std::string_view &msg);

/* Parsing utils */
int		init_password(const char *src, std::string_view &dest);
ssize_t	strlen_printable_no_spaces(const char *str);
char	to_uppercase(char c);
bool	are_equal_strs_case_insensitive(const char *str1, const size_t len1,
            const char *str2, const size_t len2);

#endif
