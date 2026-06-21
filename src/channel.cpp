#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, const std::string_view nick)
{
	for (auto &[member_ptr, flags] : channel.members)
	{
		if (are_equal_strs_case_insensitive(nick.data(), nick.size(), member_ptr->nick.data(), member_ptr->nick.size()))
			return member_ptr;
	}
	return nullptr;
}

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		channel_name(client.parser.params[0]);

	// TODO: validate channel name (# or &)
	// TODO: Reject if LIMIT (?) flag and at MAX_CHANNELS_PER_CLIENT

	// Find existing channel, or create a new one
	auto			ch_it = server.channels.find(channel_name);
	if (ch_it == server.channels.end() && !(server.channels.size() >= MAX_CHANNELS))
		ch_it = server.channels.emplace(channel_name, t_IRC_Channel{}).first;

	t_IRC_Channel	&channel = ch_it->second;

	// Set channel.name on first creation
	if (channel.name.empty())
		channel.name = channel_name;

	// Already a member -> return
	if (channel.members.find(&client) != channel.members.end())
		return;

	// TODO: reject if +i and not invited, +k and no key

	// Build member flags. first joiner becomes channel operator
	t_bmask			flags = 0;
	if (channel.members.empty())
		flags |= IS_OPERATOR;

	// Record membership on channel and client
	channel.members[&client] = flags;
	client.joined_channels.insert(&channel);

	// TODO: send response to all.
}

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string		channel_name(client.parser.params[0]);
	auto			channel_it = server.channels.find(channel_name);
	if (channel_it == server.channels.end())
	{
		//TODO: build_ERR_NOSUCHCHANNEL(client, channel_name);
		return;
	}
	t_IRC_Channel	&channel = channel_it->second;

	auto			it = channel.members.find(&client);
	if (it == channel.members.end())
	{
		//TODO: build_ERR_NOTONCHANNEL(client, channel_name);
		return;
	}

	// Record membership on channel and client
	client.joined_channels.erase(&channel);
	channel.members.erase(it);
	if (channel.members.empty())
		server.channels.erase(channel_it);
	// TODO: send response to all.
}

void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(kicker);
		return;
	}
	std::string		channel_name(kicker.parser.params[0]);
	std::string		victim(kicker.parser.params[1]);
	auto			channel_it = server.channels.find(channel_name);

	if (channel_it == server.channels.end())
	{
		// TODO: build_ERR_NOSUCHCHANNEL(kicker, channel_name);
		return;
	}
	t_IRC_Channel	&channel = channel_it->second;

	auto			kicker_it = channel.members.find(&kicker);
	if (kicker_it == channel.members.end())
	{
		// TODO: build_ERR_NOTONCHANNEL(kicker, channel_name);
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		// TODO: build_ERR_CHANOPRIVSNEEDED(kicker, channel_name);
		return;
	}

	t_IRC_Client	*to_be_kicked = find_chmember_by_nick(channel, victim);
	if (!to_be_kicked)
	{
		// TODO: build_ERR_USERNOTINCHANNEL(kicker, victim, channel_name);
		return;
	}

	// Record membership on channel and client
	to_be_kicked->joined_channels.erase(&channel);
	channel.members.erase(to_be_kicked);
	if (channel.members.empty())
		server.channels.erase(channel_it);
	// TODO: send response to all.
}
