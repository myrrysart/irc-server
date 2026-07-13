#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string_view>

void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	//No target
	if (client.parser.n_params == 0)
	{
		// no filter -> list every channel
		for (const auto &[name, channel] : server.channels)
			build_RPL_LIST(client, channel); // 322
	}
	else
	{
		// filter -> only list the requested channels
		std::string_view	targets = client.parser.params[0];
		size_t				pos = 0;

		while (pos <= targets.size())
		{
			std::string_view	name = next_comma_token(targets, pos);
			if (name.empty())
				continue; //skip empty tokens
			t_IRC_Channel	*channel = find_channel_by_name(server, name);
			if (channel)
				build_RPL_LIST(client, *channel); // 322
		}
	}
	build_RPL_LISTEND(client); // 323
}
