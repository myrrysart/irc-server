#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <string>
#include <string_view>

void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 1)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// client must be a member to see/set its topic
	std::string_view	channel_name(client.parser.params[0]);
	t_IRC_Channel		*channel = find_channel_by_name(server, channel_name);
	if (!channel)
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
		return;
	}

	auto	member_it = channel->members.find(&client);
	if (member_it == channel->members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel->name); // 442
		return;
	}

	// no second param
	if (client.parser.n_params < 2)
	{
		if (channel->topic.empty())
			build_RPL_NOTOPIC(client, channel->name); // 331
		else
			build_RPL_TOPIC(client, *channel); // 332
		return;
	}

	// +t topic change for operators only
	if (is_flag_set(channel->mode, TOPIC) && !is_flag_set(member_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel->name); // 482
		return;
	}
	channel->topic.assign(client.parser.params[1]); // copy out of the receive buffer

	std::string	line;
	append_TOPIC_msg(line, client, channel->name, channel->topic);
	broadcast_to_channel(*channel, line, client, false);
}
