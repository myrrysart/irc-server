#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <string_view>
# include <cstdint>     // fixed width data types

/* Forward declarations */
struct	s_IRC_Server;
typedef s_IRC_Server t_IRC_Server;

struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Received bytes handling */
void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
            const ssize_t received);
int		parse_message(const size_t pos, const std::string &buf,
            t_IRC_Client &client);

/* Parsing */
void	tokenize_message(t_IRC_Client &client, const std::string_view msg);

/* Parsing utils */
bool	convert_port_string_to_sixteen_bit_uint(std::string_view str, uint16_t &result);
size_t	validate_password_and_strlen(const char *str);
char	to_uppercase(char c);
size_t	skip_leading_spaces_and_check_for_empty_message(const std::string &buf,
            const size_t pos, const bool has_cr);
bool	are_equal_strs_case_insensitive(const char *str1, const size_t len1,
            const char *str2, const size_t len2);

#endif
