#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>


void	execute_KICK_cmd(t_IRC_Client &kicker, t_IRC_Server &server)
{
	if (kicker.parser.n_params < 2)
	{
		build_ERR_NEEDMOREPARAMS(kicker); // 461
		return;
	}
	std::string_view	channel_name(kicker.parser.params[0]);
	std::string_view	victims(kicker.parser.params[1]);

	std::unordered_map<std::string, t_IRC_Channel>::iterator	ch_it =
		find_channel_by_name(server, channel_name);

	if (ch_it == server.channels.end())
	{
		build_ERR_NOSUCHCHANNEL(kicker, channel_name); // 403
		return;
	}
	t_IRC_Channel	&channel = ch_it->second;

	// kicker must be in the channel and an operator
	auto			kicker_it = channel.members.find(&kicker);
	if (kicker_it == channel.members.end())
	{
		build_ERR_NOTONCHANNEL(kicker, channel.name); // 442
		return;
	}
	if (!is_flag_set(kicker_it->second, IS_OPERATOR))
	{
		build_ERR_CHANOPRIVSNEEDED(kicker, channel.name); // 482
		return;
	}

	std::string_view reason;
	if (kicker.parser.n_params >= 3) //there's a reason specified
		reason = kicker.parser.params[2];
	size_t				victim_pos = 0;
	bool				has_victim_token = false;

	while (victim_pos <= victims.size())
	{
		std::string_view	victim = next_comma_token(victims, victim_pos);
		if (victim.empty())
			continue;
		has_victim_token = true;
		trim_nickname_if_longer_than_max_nicklen(victim);

		// each victim must currently be in the channel
		t_IRC_Client	*to_be_kicked = find_chmember_by_nick(channel, victim);
		if (!to_be_kicked)
		{
			build_ERR_USERNOTINCHANNEL(kicker, channel.name, victim); // 441
			continue;
		}

		// notify, then remove.
		std::string	line;
		append_KICK_msg(line, kicker, channel.name, to_be_kicked->nick, reason);
		broadcast_to_channel(channel, line, kicker, false);
		remove_client_from_channel(*to_be_kicked, channel, server);
		// self-kick.  Channel/members may be gone, stop here
		if (to_be_kicked == &kicker)
			return;
	}
	if (!has_victim_token)
		build_ERR_NEEDMOREPARAMS(kicker); // 461
}
