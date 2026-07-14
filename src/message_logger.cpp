#include <iostream>
#include <chrono> // std::chrono::system_clock (incoming message timestamps)
#include <ctime> // std::time_t, std::tm, std::localtime()
#include <iomanip> // std::put_time() (incoming message timestamps)
#include <string_view>
#include "../lib/message_logger.hpp"

void	message_logger(const std::string_view buf,
						std::string_view direction,
						const t_IRC_Client &client)
{
	std::time_t	now = std::chrono::system_clock::to_time_t(
		std::chrono::system_clock::now());
	std::tm		*local = std::localtime(&now);
	std::cout << direction << std::put_time(local, "%H:%M:%S")
		<< " [" << client.nick << "]: "
		<< buf << std::endl;
}
