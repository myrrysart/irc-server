
#include <cctype> // for std::toupper

/* Follows CPPreference's best practice guidance regarding use of std::toupper(),
* which takes and returns an integer */
char	to_uppercase(char c)
{
	char	result =
		static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

	return (result);
}
