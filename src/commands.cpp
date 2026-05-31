#include "../lib/irc_fatstruct.hpp"

#include <iostream>

// WARN: If threads are ever introduced in this project, consider adding a flush?
// TODO: Add prefix?

// NOTE: When a messsage is unknown: Irssi (at least) returns:
// "Irssi: Unknown command: <the_command>", keeping the user's provided case.
// ----> do not output 'the_command' with a different case, remain case sensitive.
void	invalid_command_detected(const t_IRC_Client &client)
{
	// FIXME: send instead of printing.
	std::cout << "Unknown command: " << client.parser.verb << "\r\n";
}
