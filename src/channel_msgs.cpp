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

void	append_PART_msg(std::string &buf, const t_IRC_Client &who,
		std::string_view chan, std::string_view reason)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " PART ";
	buf += chan;
	if (!reason.empty())
	{
		buf += " :";
		buf += reason;
	}
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

void	append_INVITE_msg(std::string &buf, const t_IRC_Client &inviter,
		std::string_view target_nick, std::string_view chan)
{
	buf += ":";
	append_nick_user_host(buf, inviter);
	buf += " INVITE ";
	buf += target_nick;
	buf += " :";
	buf += chan;
	buf += "\r\n";
}

void	append_PRIVMSG_msg(std::string &buf, const t_IRC_Client &who,
		std::string_view target, std::string_view message)
{
	buf += ":";
	append_nick_user_host(buf, who);
	buf += " PRIVMSG ";
	buf += target;
	buf += " :";
	buf += message;
	buf += "\r\n";
}

void	build_NICK_message(std::string &nick_msg, t_IRC_Client &client,
	        const std::string &old_nick)
{
	const std::string_view	slice = " NICK ";
	size_t	len = old_nick.size() + client.username.size()
		+ sizeof(client.hostname) + slice.size() + client.nick.size() + 6;
	// 6: ':' + '!' + '@' + ':' + '\r' + '\n'

	nick_msg.reserve(len);

	nick_msg += ':';
	nick_msg += old_nick;
	nick_msg += '!';
	nick_msg += client.username;
	nick_msg += '@';
	nick_msg += client.hostname;
	nick_msg += slice;
	nick_msg += ':';
	nick_msg += client.nick;
	nick_msg += "\r\n";
}
