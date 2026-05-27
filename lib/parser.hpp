#ifndef PARSER_HPP
# define PARSER_HPP

# include "irc_fatstruct.hpp"

# include <string_view>

typedef struct	s_parser
{
	// WARN: is this even necessary
	// enum	Flags : uint8_t
	// {
	// 	HAS_TAGS     = BIT(0),
	// 	HAS_SOURCE   = BIT(1),
	// 	HAS_TRAILING = BIT(2)
	// };


	/* NOTE: max_params is set to 255, because longest message is 512 bytes,
	* the last two of those are "\r\n", and, even in an improbable scenario
	* where the message's command and each of the parameters would be only 1
	* byte long (separated by a space), we could have as many as 255 arguments. */
	static constexpr size_t	buf_size = 512;
	static constexpr size_t	max_params = 255;

	// t_bmask			state; // WARN: is this even necessary?
	//std::string_view	tags; // eventual tokens
	//std::string_view	source; // eventual tokens
	size_t				n_params; // the 'trailing' parameter is not split into differnet fields, and counts as 1
	std::string_view	cmd; // WARN: can it ONLY be one single word / 3 digits?
	std::string_view	params[max_params];

}	t_parser;
// static_assert(sizeof(t_parser) <= 65*CACHE_LINE_SIZE, "t_parser did not use 1 cache line" ); // WARN: this is huge...

void	handle_message_to_discard(t_IRC_Client &client, const char *buf,
	                              const ssize_t received);
int		prepare_and_parse_message(const size_t pos, std::string &buf,
	                              t_IRC_Client &client);
void	check_for_too_long_message(std::string &buf, t_IRC_Client &client);
void	tokenize_message(const t_IRC_Client &client, const std::string_view &msg);

#endif
