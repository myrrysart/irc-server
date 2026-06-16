#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"


void execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	//link client to channel
	std::string_view	channel_name = client.parser.params[0];
	t_IRC_Channel 		&channel = server.channels[std::string(channel_name)];
	client.joined_channels.insert(&channel);
}
