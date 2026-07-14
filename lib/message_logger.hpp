#ifndef MESSAGE_LOGGER_HPP
#define MESSAGE_LOGGER_HPP

#include <string_view>
#include "irc_fatstruct.hpp"

void	message_logger(const std::string_view buf,
						std::string_view direction,
						const t_IRC_Client &client);

#endif
