#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

#include <string>
#include <string_view>

std::string_view	next_comma_token(std::string_view list, size_t &pos)
{
	if (pos > list.size())
		return {};
	size_t			end = list.find(',', pos);
	if (end == std::string_view::npos)
		end = list.size();
	std::string_view	token = list.substr(pos, end - pos);
	pos = end + 1;
	return token;
}

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

/* helper for broadcast_to_fellow_channelers_once_per_client() */
static bool	is_message_already_loaded(t_delivery_tracker &tracker, int fd)
{
	for (size_t i = 0; i < tracker.count; ++i)
	{
		if (tracker.fds[i] == fd)
			return true;
	}
	return false;
}

/* - appends 'msg' to all buffers of clients who share a channel with 'sender'
* - skips 'sender'
* - if a client shares multiple channels with 'sender', appends 'msg' only once
* - skips clients who are about to be disconnected */
void	broadcast_to_fellow_channelers_once_per_client(t_IRC_Client &sender,
            const std::string &msg)
{
	/* To avoid duplicate alerts for clients who share more than one channel
	* with 'sender': 'tracker' stores the fds of all clients who get the alert
	* appended to their output buffers. */
	static t_delivery_tracker	tracker;
	tracker.count = 0; // reset to zero since it is a static variable with state

	// iterate through all channels 'sender' is connected to
	for (std::unordered_set<t_IRC_Channel *>::const_iterator	channel_it =
		sender.joined_channels.begin();
		channel_it != sender.joined_channels.end(); ++channel_it)
	{
		t_IRC_Channel	&channel = **channel_it;

		// iterate through all members connected to 'channel'
		for (std::unordered_map<t_IRC_Client*, t_bmask>::iterator	member_it =
			channel.members.begin(); member_it != channel.members.end();
			++member_it)
		{
			t_IRC_Client	&fellow_member = *(member_it->first);

			if (fellow_member.fd == sender.fd
				   || is_flag_set(fellow_member.state, t_IRC_Client::DISCONNECT)
				   || is_message_already_loaded(tracker, fellow_member.fd))
				continue;

			// append the alert and update 'tracker'
			fellow_member.send_message_buffer += msg;
			tracker.fds[tracker.count] = fellow_member.fd;
			++(tracker.count);
		}
	}
}
