#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "irc_fatstruct.hpp"
#include <string_view>
// commands
// /join <channel_name>
void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /part <channel_name>
void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /privmsg <target> <message>
void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /mode <channel_name> <mode>
void	execute_MODE_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /kick <channel_name> <nick>
void	execute_KICK_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /invite <nick> <channel_name>
void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /topic <channel_name> <topic>
void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /names <channel_name>
void	execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server);
// /list
void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server);
// utils
t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, std::string_view nick);
t_IRC_Channel	*find_channel_by_name(t_IRC_Server &server, std::string_view ch_name);
t_IRC_Client	*find_client_by_nick(t_IRC_Server &server, std::string_view nick);
void			remove_client_from_channel(t_IRC_Client &client, t_IRC_Channel &channel, t_IRC_Server &server);
void			broadcast_to_channel(t_IRC_Channel &channel, const std::string &line, t_IRC_Client &client, bool skip_sender);
std::string_view	next_comma_token(std::string_view list, size_t &pos);
void			broadcast_to_fellow_channelers_once_per_client(t_IRC_Client &sender, const std::string &msg);
// message builders
void	append_JOIN_msg(std::string &buf, const t_IRC_Client &who, std::string_view chan);
void	append_PART_msg(std::string &buf, const t_IRC_Client &who,
			std::string_view chan, std::string_view reason);
void	append_KICK_msg(std::string &buf, const t_IRC_Client &kicker,
			std::string_view chan, std::string_view victim_nick, std::string_view reason);
void	append_MODE_msg(std::string &buf, const t_IRC_Client &who, std::string_view chan, std::string_view mode);
void	append_TOPIC_msg(std::string &buf, const t_IRC_Client &who,
		std::string_view chan, std::string_view topic);
void	append_INVITE_msg(std::string &buf, const t_IRC_Client &inviter,
		std::string_view target_nick, std::string_view chan);
void	append_PRIVMSG_msg(std::string &buf, const t_IRC_Client &who,
		std::string_view target, std::string_view message);
void	send_names_reply(t_IRC_Client &client, const t_IRC_Channel &channel);

typedef struct	s_delivery_tracker
{
	int			fds[MAX_CLIENTS];
	size_t		count;
}				t_delivery_tracker;

#endif
