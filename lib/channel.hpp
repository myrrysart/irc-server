#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "irc_fatstruct.hpp"

void	execute_JOIN_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_PART_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_PRIVMSG_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_MODE_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_KICK_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_INVITE_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_TOPIC_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_NAMES_cmd(t_IRC_Client &client, t_IRC_Server &server);
void	execute_LIST_cmd(t_IRC_Client &client, t_IRC_Server &server);
void 	execute_USER_cmd(t_IRC_Client &client);

#endif
