#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <string_view>
# include <charconv>     // std::from_chars(), std::from_chars_result
# include <system_error> // std::errc

/* Forward declarations */
struct	s_IRC_Server;
typedef s_IRC_Server t_IRC_Server;

struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Received bytes handling */
void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
            ssize_t received);
int		parse_message(size_t pos, const std::string &buf, t_IRC_Client &client);

/* Parsing */
void	tokenize_message(t_IRC_Client &client, std::string_view msg);

/* Parsing utils */
size_t	validate_password_and_strlen(const char *str);
char	to_uppercase(char c);
size_t	skip_leading_spaces_and_check_for_empty_message(const std::string &buf,
            size_t pos, bool has_cr);
bool	are_equal_strs_case_insensitive(std::string_view str1, std::string_view str2);
void	trim_nickname_if_longer_than_max_nicklen(std::string_view &nick);

template <typename UIntType> // expects unsigned integers only!
bool	parse_positive_integer_and_validate_input(std::string_view str, UIntType &val)
{
	const char* const	begin = str.data();
	const char* const	end = str.data() + str.size();

	std::from_chars_result	result = std::from_chars(begin, end, val);

	if (result.ec != std::errc{} || result.ptr != end || val == 0)
		return false;
	return true;
}

#endif
