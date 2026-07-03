#include "../lib/channel.hpp"
#include "../lib/commands.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include <string>
#include <string_view>

void	append_JOIN_msg(std::string &buf, const t_IRC_Client &who, std::string_view chan)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " JOIN ";
	buf += chan;
	buf += "\r\n";
}

void	append_PART_msg(std::string &buf, const t_IRC_Client &who, std::string_view chan)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " PART ";
	buf += chan;
	buf += "\r\n";
}

void	append_KICK_msg(std::string &buf, const t_IRC_Client &kicker,
		std::string_view chan, std::string_view victim_nick, std::string_view reason)
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

void	append_MODE_msg(std::string &buf, const t_IRC_Client &who,  std::string_view chan, std::string_view mode)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " MODE ";
	buf += chan;
	buf += " ";
	buf += mode;
	buf += "\r\n";
}

void append_TOPIC_msg(std::string &buf, const t_IRC_Client &who,
		std::string_view chan, std::string_view topic)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " TOPIC ";
	buf += chan;
	buf += " :";
	buf += topic;
	buf += "\r\n";
}
