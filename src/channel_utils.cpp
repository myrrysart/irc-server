#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

#include <string>

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, const std::string_view nick)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (are_equal_strs_case_insensitive(nick.data(), nick.size(), member_ptr->nick.data(), member_ptr->nick.size()))
			return member_ptr;
	}
	return nullptr;
}

t_IRC_Channel	*find_channel_by_name(t_IRC_Server &server, const std::string &ch_name)
{
	if (auto name_it = server.channels.find(ch_name); name_it != server.channels.end())
		return &name_it->second;
	return nullptr;
}

void	remove_client_from_channel(t_IRC_Client &client, t_IRC_Channel &channel, t_IRC_Server &server)
{
	client.joined_channels.erase(&channel);
	channel.members.erase(&client);
	channel.invited.erase(&client);
	if (channel.members.empty())
		server.channels.erase(channel.name);
}

 t_IRC_Client	*find_client_by_nick(t_IRC_Server &server, const std::string_view nick)
{
	for (auto &[fd, client] : server.clients)
	{
		(void)fd;
		if (are_equal_strs_case_insensitive(
				nick.data(), nick.size(),
				client.nick.data(), client.nick.size()))
			return &client;
	}
	return nullptr;
}

void	broadcast_to_channel(t_IRC_Channel &channel, const std::string &line, t_IRC_Client &client, bool skip_sender)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (skip_sender && member_ptr == &client)
			continue;
		member_ptr->send_message_buffer += line;

	}
}
