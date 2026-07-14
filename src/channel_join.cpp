#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

static void	handle_join_request_to_new_channel(t_IRC_Client &client,
	            t_IRC_Server &server, std::string_view channel_name);
static void	handle_join_request_to_existing_channel(t_IRC_Client &client,
	            std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it,
	            const std::string &channel_name, std::string_view key);
static void	build_JOIN_reply_and_broadcast(t_IRC_Client &client,
	            t_IRC_Channel &channel);

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// "0" is a special JOIN argument
	if (client.parser.params[0] == "0")
	{
		remove_client_from_all_channels(client, server);
		return;
	}

	std::string_view	channels(client.parser.params[0]);
	std::string_view	keys;
	size_t				channel_pos = 0;
	size_t				key_pos = 0;

	// if there are more params than one, we consider them keys for the channels
	if (client.parser.n_params >= 2)
		keys = client.parser.params[1];
	// each parsed request pairs one channel token with the next key token (if any)
	while (channel_pos <= channels.size())
	{
		// directly initialize the struct's members, without intermediate steps
		t_key_channel	req{ next_comma_token(channels, channel_pos), {} };
		// key/channel pairing — RFC pairs keys by index for every
		// channel in JOIN ch1,ch2 k1,k2
		// Even if the channel is an invalid one, the pairing still holds.
		if (client.parser.n_params >= 2 && key_pos <= keys.size())
			req.key = next_comma_token(keys, key_pos);

		if (req.channel.size() < 2 || (req.channel[0] != '#' && req.channel[0] != '&')
			|| req.channel.size() > t_IRC_Channel::CHANNELLEN)
		{
			build_ERR_BADCHANMASK(client, req.channel); // 476
			continue;
		}

		std::string	channel_name(req.channel);

		std::unordered_map<std::string, t_IRC_Channel>::iterator	ch_it =
			find_channel_by_name(server, channel_name);

		if (ch_it == server.channels.end())
			handle_join_request_to_new_channel(client, server, channel_name);
		else // existing channel
			handle_join_request_to_existing_channel(client, ch_it, channel_name, req.key);
	}
}

static void	handle_join_request_to_new_channel(t_IRC_Client &client,
	            t_IRC_Server &server, std::string_view channel_name)
{
	// doesn't exist yet. Create it, first joiner becomes operator
	// but first: check for channel limits
	if (server.channels.size() >= MAX_CHANNELS)
	{
		build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
		return;
	}
	if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
	{
		build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
		return;
	}

	std::unordered_map<std::string, t_IRC_Channel>::iterator	ch_it =
		server.channels.emplace(channel_name, t_IRC_Channel{}).first;

	t_IRC_Channel	&channel = ch_it->second;
	channel.name = channel_name;
	channel.mode |= TOPIC;
	channel.members.emplace(&client, IS_OPERATOR);

	client.joined_channels.insert(&channel);
	channel.invited.erase(&client); // consume any pending invite

	build_JOIN_reply_and_broadcast(client, channel);
}

static void	handle_join_request_to_existing_channel(t_IRC_Client &client,
	            std::unordered_map<std::string, t_IRC_Channel>::iterator ch_it,
	            const std::string &channel_name, std::string_view key)
{
	// already in it. Silent.
	if (ch_it->second.members.contains(&client))
		return;

	// check all flags

	if (client.joined_channels.size() >= MAX_CHANNELS_PER_CLIENT)
	{
		build_ERR_TOOMANYCHANNELS(client, channel_name); // 405
		return;
	}

	if (is_flag_set(ch_it->second.mode, LIMIT)
		&& ch_it->second.members.size() >= ch_it->second.user_limit)
	{
		build_ERR_CHANNELISFULL(client, ch_it->second.name); // 471
		return;
	}

	if (is_flag_set(ch_it->second.mode, INVITE)
		&& !ch_it->second.invited.contains(&client))
	{
		build_ERR_INVITEONLYCHAN(client, ch_it->second.name); // 473
		return;
	}

	if (is_flag_set(ch_it->second.mode, KEY))
	{
		if (key.empty() || ch_it->second.key != key)
		{
			build_ERR_BADCHANNELKEY(client, ch_it->second.name); // 475
			return;
		}
	}

	t_IRC_Channel	&channel = ch_it->second;

	channel.members.emplace(&client, 0);
	client.joined_channels.insert(&channel);
	channel.invited.erase(&client); // consume any pending invite

	// all checks passed. record membership and broadcast
	build_JOIN_reply_and_broadcast(client, channel);
}

static void	build_JOIN_reply_and_broadcast(t_IRC_Client &client,
	            t_IRC_Channel &channel)
{
	std::string		line;

	append_JOIN_msg(line, client, channel.name);
	broadcast_to_channel(channel, line, client, false);
	if (!channel.topic.empty())
		build_RPL_TOPIC(client, channel); // 332
	send_names_reply(client, channel);
}
