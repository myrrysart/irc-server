#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <string>
#include <string_view>
#include <unordered_map>

void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	if (client.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(client); // 461
		return;
	}

	// find channel, inviter must be a member and an operator
	std::string_view	target_nick(client.parser.params[0]);
	trim_nickname_if_longer_than_max_nicklen(target_nick);
	std::string_view	channel_name(client.parser.params[1]);
	std::unordered_map<std::string, t_IRC_Channel>::iterator	ch_it =
		find_channel_by_name(server, channel_name);

	if (ch_it == server.channels.end())
	{
		build_ERR_NOSUCHCHANNEL(client, channel_name); // 403
		return;
	}
	t_IRC_Channel	&channel = ch_it->second;

	auto	inviter_it = channel.members.find(&client);
	if (inviter_it == channel.members.end())
	{
		build_ERR_NOTONCHANNEL(client, channel.name); // 442
		return;
	}
	if (!is_flag_set(inviter_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(client, channel.name); // 482
		return;
	}

	// target must exist and not already be a member
	t_IRC_Client *target = find_client_by_nick(server, target_nick);
	if (!target)
	{
		build_ERR_NOSUCHNICK(client, target_nick); // 401
		return;
	}

	if (channel.members.contains(target))
	{
		build_ERR_USERONCHANNEL(client, target->nick, channel.name); // 443
		return;
	}

	// record invite (bypasses +i) and notify both sides
	channel.invited.insert(target);
	build_RPL_INVITING(client, target->nick, channel.name); // 341

	std::string		line;
	append_INVITE_msg(line, client, target->nick, channel.name);
	target->send_message_buffer += line;
}
