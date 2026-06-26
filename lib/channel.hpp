#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "irc_fatstruct.hpp"
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

void 	execute_USER_cmd(t_IRC_Client &client);

t_IRC_Client	*find_chmember_by_nick(t_IRC_Channel &channel, const std::string_view nick);

#endif
