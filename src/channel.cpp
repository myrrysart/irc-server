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
	std::string_view	channel_name = client.parser.params[0];
	t_IRC_Channel 		&channel = server.channels[std::string(channel_name)];
	channel.members[&client] = 0; //init flags
	if (channel.members.empty())
		channel.members[&client]|= IS_OPERATOR; //if first client -> operator
	client.joined_channels.insert(&channel);
}


void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(client);
		return;
	}
	std::string_view channel_name = client.parser.params[0];
	t_IRC_Channel &channel = server.channels[std::string(channel_name)];
	client.joined_channels.erase(&channel);
	if (channel.members.size() <= 1)
		server.channels.erase(std::string(channel_name)); //the string conversion here might not be needed in C++20
}
