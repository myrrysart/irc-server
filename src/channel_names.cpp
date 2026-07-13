#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string_view>

void execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no params -> broadcast every channel on the server
	if (client.parser.n_params == 0)
	{
		auto	channel_it = server.channels.begin();
		while (channel_it != server.channels.end())
		{
			send_names_reply(client, channel_it->second);
			++channel_it;
		}
		if (server.channels.empty())
			build_RPL_ENDOFNAMES(client, "*"); // 366
		return;
	}

	// params only the requested channels -> unknown ones get ENDOFNAMES
	std::string_view	channels(client.parser.params[0]);
	size_t				channel_pos = 0;
	bool				saw_nonempty_channel_toke = false;

	while (channel_pos <= channels.size())
	{
		std::string_view	channel_name = next_comma_token(channels, channel_pos);
		if (channel_name.empty())
			continue;
		saw_nonempty_channel_toke = true;

		t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
		if (!channel)
		{
			build_RPL_ENDOFNAMES(client, channel_name); // 366 this is supposedly valid, but don't show on irssi
			continue;
		}
		send_names_reply(client, *channel);
	}
	if (!saw_nonempty_channel_toke)
		build_RPL_ENDOFNAMES(client, "*"); // 366 this is supposedly valid, but don't show on irssi
}
