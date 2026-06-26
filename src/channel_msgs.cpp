#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <string>

void	append_JOIN_msg(std::string &buf, const t_IRC_Client &who, const std::string &chan)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " JOIN ";
	buf += chan;
	buf += "\r\n";
}

void	append_PART_msg(std::string &buf, const t_IRC_Client &who, const std::string &chan)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " PART ";
	buf += chan;
	buf += "\r\n";
}

void	append_KICK_msg(std::string &buf, const t_IRC_Client &kicker,
		const std::string &chan, std::string_view victim_nick, std::string_view reason)
{
	buf += ":";
	append_nick_user_host(buf, kicker);
	buf += " KICK ";
	buf += chan;
	buf += ' ';
	buf += victim_nick;
	if (!reason.empty())
	{
		buf += " :";
		buf += reason;
	}
	buf += "\r\n";
}

void	append_MODE_msg(std::string &buf, const t_IRC_Client &who, const std::string &chan, const std::string &mode)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " MODE ";
	buf += chan;
	buf += " ";
	buf += mode;
	buf += "\r\n";
}

void	append_TOPIC_msg(std::string &buf, const t_IRC_Client &who, std::string_view topic)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " TOPIC ";
	buf += topic;
	buf += "\r\n";
}

void	append_NAMES_reply(t_IRC_Client &client, std::string_view line)
{
	std::string	&buf = client.send_message_buffer;
	append_common_reply_prefix(buf, "353", client.nick);
	buf += " = ";
	buf += line;
	buf += "\r\n";
}
