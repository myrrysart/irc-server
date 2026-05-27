#ifndef PARSER_HPP
# define PARSER_HPP

# include <string_view>

typedef struct	s_tokenizer
{
	// WARN: is this even necessary
	// enum	Masks : uint8_t
	// {
	// 	HAS_TAGS		BIT(0),
	// 	HAS_SOURCE		BIT(1),
	// 	HAS_TRAILING	BIT(2),
	// 	MSG_TOO_LONG	BIT(3)
	// };


	/* NOTE: max_params is set to 255, because longest message is 512 bytes,
	* the last two of those are "\r\n", and, even in an improbable scenario
	* where the message's command and each of the parameters would be only 1
	* byte long (separated by a space), we could have as many as 255 arguments. */
	static constexpr std::size_t	max_params = 255;


	// t_bmask		state; // WARN: is this even necessary?

	//std::string_view	tags;
	//std::string_view	source;
	std::size_t			n_params; // the 'trailing' parameter is not split into differnet fields, and counts as 1
	std::string_view	cmd; // WARN: can it ONLY be one single word / 3 digits?
	std::string_view	params[max_params];

}	t_tokenizer;

// FIXME: Can messages contain more than one space between tokens?

// FIXME: When a received message is longer than 512 bytes: remember to DISCARD
// the rest of the message that follows the truncation.

// FIXME: What if received message does not end with either "\r\n" or '\n', even
// below 512 bytes? It is possible to achieve: netcat Client can send
// unterminated messages with Ctrl+D, thus closing stdin.

#endif
