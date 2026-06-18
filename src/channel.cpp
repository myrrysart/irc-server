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
	std::string		channel_name(client.parser.params[0]);
	t_IRC_Channel 	&channel = server.channels[channel_name];

	if (channel.members.size() >= static_cast<size_t>(channel.user_limit))
	{
		// TODO: build_ERR_CHANNELFULL(client);
		return;
	}
	channel.members[&client] = 0; //init flags
	if (channel.members.empty())
		channel.members[&client]|= IS_OPERATOR; //if first client -> operator
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
	t_IRC_Channel	&channel = server.channels[channel_name];

	auto			it = channel.members.find(&client);
	if (it == channel.members.end())
	{
		//TODO: reply: couldn't find user.
		return;
	}
	client.joined_channels.erase(&channel);
	channel.members.erase(it);
	if (channel.members.empty())
		server.channels.erase(channel_name); //the string conversion here might not be needed in C++20
}

void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params == 0)
	{
		build_ERR_NEEDMOREPARAMS(kicker);
		return;
	}
	std::string		channel_name(kicker.parser.params[0]);
	std::string		nick_to_be_kicked(kicker.parser.params[1]);
	t_IRC_Channel	&channel = server.channels[channel_name];
	t_IRC_Client 	*to_be_kicked = nullptr;

	if (!(is_flag_set(channel.members[&kicker], IS_OPERATOR)))
	{
		// build_ERR_CHANOPRIVSNEEDED(kicker);
		return;
	}

	for (auto &[member_ptr, flags] : channel.members)
	{
		if (member_ptr->nick == nick_to_be_kicked)
		{
			to_be_kicked = member_ptr;
			break;
		}
	}

	if (!to_be_kicked)
	{
		// send ERR_USERNOTINCHANNEL reply
		return;
	}

	to_be_kicked->joined_channels.erase(&channel);
	channel.members.erase(to_be_kicked);
	if (channel.members.empty())
		server.channels.erase(channel_name);
}

// void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server)
// {
// 	if (client.parser.n_params == 0)
// 	{
// 		build_ERR_NEEDMOREPARAMS(client);
// 		return;
// 	}
// 	std::string		channel_name(client.parser.params[0]);
// 	t_IRC_Channel	&channel = server.channels[channel_name];
// }
