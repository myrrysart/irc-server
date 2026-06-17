#include "../lib/irc_fatstruct.hpp"
#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"

//TODO: messages back to client(s)

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
	//link channel to client
	client.joined_channels.insert(&channel);
	channel.members[&client] = 0; //initial flags (no flags)
	//TODO: might need the operator flag up if first on channel?
}

// bit manipulations
// channel.members[&client] = (channel.members.empty()) ? IS_OPERATOR : 0;
