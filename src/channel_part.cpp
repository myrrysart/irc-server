#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>

void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}
	std::string_view	channels(client.parser.params[0]);
	std::string_view	reason;
	size_t				channel_pos = 0;
	bool				has_nonempty_channel_token = false;

	if (client.parser.n_params >= 2)
		reason = client.parser.params[1];
	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		has_nonempty_channel_token = true;

		// channel must exist and client must be in it
		t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
		if (!channel)
		{
			build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
			continue;
		}

		if (!channel->members.contains(&client))
		{
			build_ERR_NOTONCHANNEL(client, channel->name); // 442
			continue;
		}

		// broadcast to  members, then erase membership
		std::string		line;
		append_PART_msg(line, client, channel->name, reason);
		broadcast_to_channel(*channel, line, client, false);
		remove_client_from_channel(client, *channel, server);
	}
	if (!has_nonempty_channel_token)
		build_ERR_NEEDMOREPARAMS(client); // 461
}
