#ifndef NUMERICS_HPP
# define NUMERICS_HPP

# include <string_view>

/* Forward declarations */
struct	s_IRC_Server;
typedef s_IRC_Server t_IRC_Server;

struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Numeric Replies */
void	build_RPL_WELCOME(t_IRC_Client &client,                      // 001
            t_IRC_Server &server);
// void	send_RPL_YOURHOST(const t_IRC_Client &client);               // 002 // WARN: is it used
int		build_ERR_INPUTTOOLONG(t_IRC_Client &client);                // 417
void	build_ERR_UNKNOWNCOMMAND(t_IRC_Client &client,               // 421
            t_IRC_Server &server);
void	build_ERR_NONICKNAMEGIVEN(t_IRC_Client &client,              // 431
            t_IRC_Server &server);
void	build_ERR_ERRONEOUSNICKNAME(t_IRC_Client &client,            // 432
            const std::string_view &new_nick, t_IRC_Server &server);
void	build_ERR_NICKNAMEINUSE(t_IRC_Client &client,                // 433
            const std::string_view &new_nick, t_IRC_Server &server);
void	build_ERR_NOTREGISTERED(t_IRC_Client &client);               // 451
void	build_ERR_NEEDMOREPARAMS(t_IRC_Client &client);              // 461
void	build_ERR_ALREADYREGISTERED(t_IRC_Client &client);           // 462
void	build_ERR_PASSWDMISMATCH(t_IRC_Client &client);              // 464

#endif
