#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

#include <string>
#include <string_view>

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, std::string_view nick)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (are_equal_strs_case_insensitive(nick.data(), nick.size(), member_ptr->nick.data(), member_ptr->nick.size()))
			return member_ptr;
	}
	return nullptr;
}

t_IRC_Channel	*find_channel_by_name(t_IRC_Server &server, std::string_view ch_name)
{
	auto	name_it = server.channels.find(std::string(ch_name));
	if (name_it != server.channels.end())
		return &name_it->second;
	return nullptr;
}

t_IRC_Client	*find_client_by_nick(t_IRC_Server &server, std::string_view nick)
{
	for (auto &entry : server.clients)
	{
		t_IRC_Client	&client = entry.second;
		if (are_equal_strs_case_insensitive(
				nick.data(), nick.size(),
				client.nick.data(), client.nick.size()))
			return &client;
	}
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

void	broadcast_to_channel(t_IRC_Channel &channel, const std::string &line, t_IRC_Client &client, bool skip_sender)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (skip_sender && member_ptr == &client)
			continue;
		member_ptr->send_message_buffer += line;

	}
}

void	send_names_reply(t_IRC_Client &client, const t_IRC_Channel &channel)
{
	std::string	names;

	for (const auto &[member, member_flags] : channel.members)
	{
		if (is_flag_set(member_flags, IS_OPERATOR))
			names += '@';
		names += member->nick;
		names += ' ';
	}
	if (!names.empty())
		names.pop_back(); //getting rid of the space at the end
	build_RPL_NAMREPLY(client, channel.name, names);
	build_RPL_ENDOFNAMES(client, channel.name);
}
