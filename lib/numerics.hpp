#ifndef NUMERICS_HPP
# define NUMERICS_HPP

# include <string_view>

/* Forward declarations */
struct	s_IRC_Server;
typedef s_IRC_Server t_IRC_Server;

struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

struct	s_IRC_Channel;
typedef s_IRC_Channel t_IRC_Channel;

/* Numeric Replies */
void	build_RPL_WELCOME(t_IRC_Client &client);                         // 001
void	build_RPL_YOURHOST(t_IRC_Client &client);                        // 002
void	build_RPL_CREATED(t_IRC_Client &client);                         // 003
void	build_RPL_MYINFO(t_IRC_Client &client);                          // 004
void	build_RPL_ISUPPORT(t_IRC_Client &client);                        // 005
void	build_RPL_UMODEIS(t_IRC_Client &client);                         // 221
void	build_RPL_LIST(t_IRC_Client &client,                             // 322
			const t_IRC_Channel &channel);
void	build_RPL_LISTEND(t_IRC_Client &client);                         // 323
void	build_RPL_CHANNELMODEIS(t_IRC_Client &client,                    // 324
            const t_IRC_Channel &channel);
void	build_RPL_NOTOPIC(t_IRC_Client &client,                          // 331
			std::string_view channel);
void	build_RPL_TOPIC(t_IRC_Client &client,                            // 332
			const t_IRC_Channel &channel);
void	build_RPL_INVITING(t_IRC_Client &client,                         // 341
			std::string_view target_nick, std::string_view channel_name);
void	build_RPL_NAMREPLY(t_IRC_Client &client,                         // 353
			std::string_view channel, std::string_view line);
void	build_RPL_ENDOFNAMES(t_IRC_Client &client,                       // 366
			std::string_view channel);
void	build_RPL_MOTD(t_IRC_Client &client,                             // 372
			std::string_view line);
void	build_RPL_MOTDSTART(t_IRC_Client &client);                       // 375
void	build_RPL_ENDOFMOTD(t_IRC_Client &client);                       // 376


/* Error messages */
void	build_ERR_NOSUCHNICK(t_IRC_Client &client,                       // 401
            std::string_view nick);
void	build_ERR_NOSUCHCHANNEL(t_IRC_Client &client,                    // 403
            std::string_view channel);
void	build_ERR_CANNOTSENDTOCHAN(t_IRC_Client &client,                 // 404
            std::string_view channel);
void	build_ERR_TOOMANYCHANNELS(t_IRC_Client &client,                  // 405
            std::string_view channel);
void	build_ERR_NOORIGIN(t_IRC_Client &client);                        // 409
void	build_ERR_NORECIPIENT(t_IRC_Client &client);                     // 411
void	build_ERR_NOTEXTTOSEND(t_IRC_Client &client);                    // 412
void	build_ERR_INPUTTOOLONG(t_IRC_Client &client);                    // 417
void	build_ERR_UNKNOWNCOMMAND(t_IRC_Client &client);                  // 421
void	build_ERR_NOMOTD(t_IRC_Client &client);                          // 422
void	build_ERR_NONICKNAMEGIVEN(t_IRC_Client &client);                 // 431
void	build_ERR_ERRONEOUSNICKNAME(t_IRC_Client &client,                // 432
            std::string_view new_nick);
void	build_ERR_NICKNAMEINUSE(t_IRC_Client &client,                    // 433
            std::string_view new_nick);
void	build_ERR_USERNOTINCHANNEL(t_IRC_Client &client,                 // 441
            std::string_view channel, std::string_view nick);
void	build_ERR_NOTONCHANNEL(t_IRC_Client &client,                     // 442
            std::string_view channel);
void	build_ERR_USERONCHANNEL(t_IRC_Client &client,                    // 443
            std::string_view nick, std::string_view channel);
void	build_ERR_NOTREGISTERED(t_IRC_Client &client);                   // 451
void	build_ERR_NEEDMOREPARAMS(t_IRC_Client &client);                  // 461
void	build_ERR_ALREADYREGISTERED(t_IRC_Client &client);               // 462
void	build_ERR_PASSWDMISMATCH(t_IRC_Client &client);                  // 464
void	build_ERR_CHANNELISFULL(t_IRC_Client &client,                    // 471
			std::string_view channel);
void	build_ERR_UNKNOWNMODE(t_IRC_Client &client,                      // 472
			char mode_char);
void	build_ERR_INVITEONLYCHAN(t_IRC_Client &client,                   // 473
			std::string_view channel);
void	build_ERR_BADCHANNELKEY(t_IRC_Client &client,                    // 475
			std::string_view channel);
void	build_ERR_BADCHANMASK(t_IRC_Client &client,                      // 476
			std::string_view channel);
void	build_ERR_CHANOPRIVSNEEDED(t_IRC_Client &client,                 // 482
            std::string_view channel);
void	build_ERR_UMODEUNKNOWNFLAG(t_IRC_Client &client);                // 501
void	build_ERR_USERSDONTMATCH(t_IRC_Client &client);                  // 502

/* Utils */
void	append_common_reply_prefix(std::string &buffer,
            std::string_view numeric, std::string_view nick);

#endif
