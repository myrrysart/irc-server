#include "../lib/irc_fatstruct.hpp"
#include "../lib/parser.hpp"

#include <cctype>       // for std::toupper() & std::iscntrl()
#include <string>
#include <string_view>
#include <charconv>     // for std::from_chars() and std::from_chars_result
#include <system_error> // for std::errc (from_chars() error codes)

/* exception-free numeric parsing function.
*  Accepts the range [1, UINT16_MAX] inclusive, suitable for a port.
*  Accepting port 0 would be asking from the kernel to grant any available port.
*  Rejects any non-numeric character. */
bool	convert_port_string_to_sixteen_bit_uint(std::string_view str, uint16_t &result)
{
	std::from_chars_result	conversion_info =
		std::from_chars(str.cbegin(), str.cend(), result);

	if (conversion_info.ptr != str.cend() ||
			conversion_info.ec == std::errc::invalid_argument ||
			conversion_info.ec == std::errc::result_out_of_range ||
			result == 0)
		return false;
	return true;
}

/* Return values:
 * • The length of the string
 * • 0 if any control characters or spaces are detected */
size_t	validate_password_and_strlen(const char *str)
{
	ssize_t	i = 0;

	for ( ; str[i]; ++i)
	{
		if (std::iscntrl(static_cast<unsigned char>(str[i])) || str[i] == ' ')
			return (0);
	}
	return (i);
}

/* Follows CPPreference's best practice guidance regarding use of std::toupper(),
* which takes and returns an integer */
char	to_uppercase(char c)
{
	char	result =
		static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

	return (result);
}

size_t	skip_leading_spaces_and_check_for_empty_message(const std::string &buf,
            size_t pos, bool has_cr)
{
	size_t	i = 0;

	// skip leading spaces
	while (i < pos && buf[i] == ' ')
		++i;

	/* returns true if the first message in the buffer is empty (i.e. containing only):
	* • LF ('\n')
	* • CRLF ("\r\n")
	* • spaces culminating with LF or CRLF */
	if (i == pos - has_cr)
		return (std::string::npos);

	return i;
}

bool	are_equal_strs_case_insensitive(std::string_view str1, std::string_view str2)
{
	if (str1.size() != str2.size())
		return false;

	for (size_t i = 0; i < str1.size(); ++i)
	{
		if (to_uppercase(str1[i]) != to_uppercase(str2[i]))
			return false;
	}
	return true;
}

/* 'nick' has to be passed as a reference */
void	trim_nickname_if_longer_than_max_nicklen(std::string_view &nick)
{
	if (nick.size() > t_IRC_Client::max_nicklen)
		nick.remove_suffix(nick.size() - t_IRC_Client::max_nicklen);
}
