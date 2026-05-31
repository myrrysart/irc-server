#include "../lib/parser.hpp"

#include <cctype> // for std::toupper() & std::iscntrl()
#include <string_view>

int	init_password(const char *src, std::string_view &dest)
{
	ssize_t	len = strlen_printable_no_spaces(src);

	if (len <= 0)
		return (-1);

	dest = std::string_view{src, static_cast<size_t>(len)};
	return (0);
}

/* Return values:
 * • The length of the string
 * • -1 if any control characters or spaces are detected */
ssize_t	strlen_printable_no_spaces(const char *str)
{
	ssize_t	i = 0;

	for ( ; str[i]; ++i)
	{
		if (std::iscntrl(static_cast<unsigned char>(str[i])) || str[i] == ' ')
			return (-1);
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
