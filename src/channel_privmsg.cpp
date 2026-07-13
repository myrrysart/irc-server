#include "../lib/channel.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <cstddef>
#include <string>
#include <string_view>

void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no target at all
	if (client.parser.n_params == 0)
	{
		build_ERR_NORECIPIENT(client); // 411
		return;
	}
	// WARN: n_params==1 cannot distinguish PRIVMSG bob (412) from PRIVMSG :hello (411);
	// Same param, so needs parser input to return 411 for trailing-only lines.
	// Is this needed in practice? Irssi invokes just 412
	// target given but no message text
	if (client.parser.n_params < 2)
	{
		build_ERR_NOTEXTTOSEND(client); // 412
		return;
	}

	std::string_view	targets(client.parser.params[0]);
	std::string_view	message(client.parser.params[1]);
	size_t				pos = 0;
	bool				any_message_delivered = false;

	while (pos <= targets.size())
	{
		std::string_view	target = next_comma_token(targets, pos);
		if (target.empty())
			continue;

		// channel target: must exist and client must be a member
		if (target[0] == '#' || target[0] == '&')
		{
			std::unordered_map<std::string, t_IRC_Channel>::iterator	ch_it =
				find_channel_by_name(server, target);
			if (ch_it == server.channels.end())
			{
				build_ERR_NOSUCHCHANNEL(client, target); // 403
				return;
			}
			t_IRC_Channel	&channel = ch_it->second;
			if (!channel.members.contains(&client))
			{
				build_ERR_CANNOTSENDTOCHAN(client, target); // 404
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, channel.name, message);
			broadcast_to_channel(channel, line, client, true);
		}
		// nick target: must be an online client
		else
		{
			trim_nickname_if_longer_than_max_nicklen(target);
			t_IRC_Client	*target_client = find_client_by_nick(server, target);
			if (!target_client)
			{
				build_ERR_NOSUCHNICK(client, target); // 401
				return;
			}
			std::string		line;
			append_PRIVMSG_msg(line, client, target_client->nick, message);
			target_client->send_message_buffer += line;
		}
		any_message_delivered = true;
	}
	// catches "targets was just commas" (e.g. PRIVMSG , :hi)
	if (!any_message_delivered)
		build_ERR_NORECIPIENT(client); // 411
}
