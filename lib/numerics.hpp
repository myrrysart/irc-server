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
void	build_RPL_WELCOME(t_IRC_Client &client);                     // 001
void	build_RPL_YOURHOST(t_IRC_Client &client);                    // 002
void	build_RPL_CREATED(t_IRC_Client &client);                   // 003
void	build_RPL_MYINFO(t_IRC_Client &client);                      // 004
void	build_RPL_ISUPPORT(t_IRC_Client &client);                    // 005
void	build_RPL_CHANNELMODEIS(t_IRC_Client &client,                 // 324
            const t_IRC_Channel &channel);
void	build_RPL_NAMES(t_IRC_Client &client, const std::string_view line); // 353
void	build_RPL_ENDOFNAMES(t_IRC_Client &client, const std::string_view channel);  // 366
void	build_RPL_LIST(t_IRC_Client &client, const t_IRC_Channel &channel); // 322
void	build_RPL_LISTEND(t_IRC_Client &client);                    // 323

void	build_ERR_NOSUCHNICK(t_IRC_Client &client,                   // 401
            const std::string_view nick);
void	build_ERR_NOSUCHCHANNEL(t_IRC_Client &client,                // 403
            const std::string_view channel);
void	build_ERR_CANNOTSENDTOCHAN(t_IRC_Client &client,             // 404
            const std::string_view channel);
void	build_ERR_TOOMANYCHANNELS(t_IRC_Client &client,              // 405
            const std::string_view channel);
void	build_ERR_NOMOTD(t_IRC_Client &client);                      // 422
void	build_ERR_INPUTTOOLONG(t_IRC_Client &client);                // 417
void	build_ERR_UNKNOWNCOMMAND(t_IRC_Client &client);              // 421
void	build_ERR_NONICKNAMEGIVEN(t_IRC_Client &client);             // 431
void	build_ERR_ERRONEOUSNICKNAME(t_IRC_Client &client,            // 432
            const std::string_view new_nick);
void	build_ERR_NICKNAMEINUSE(t_IRC_Client &client,                // 433
            const std::string_view new_nick);
void	build_ERR_USERNOTINCHANNEL(t_IRC_Client &client,             // 441
            const std::string_view channel, const std::string_view nick);
void	build_ERR_NOTONCHANNEL(t_IRC_Client &client,                 // 442
            const std::string_view channel);
void	build_ERR_NOTREGISTERED(t_IRC_Client &client);               // 451
void	build_ERR_NEEDMOREPARAMS(t_IRC_Client &client);              // 461
void	build_ERR_ALREADYREGISTERED(t_IRC_Client &client);           // 462
void	build_ERR_PASSWDMISMATCH(t_IRC_Client &client);              // 464
void	build_ERR_CHANNELISFULL(t_IRC_Client &client,                // 471
            const std::string_view channel);
void	build_ERR_UNKNOWNMODE(t_IRC_Client &client,                // 472
            const std::string_view channel, char mode_char);
void	build_ERR_INVITEONLYCHAN(t_IRC_Client &client,               // 473
            const std::string_view channel);
void	build_ERR_BADCHANNELKEY(t_IRC_Client &client,                // 475
            const std::string_view channel);
void	build_ERR_BADCHANMASK(t_IRC_Client &client,                  // 476
            const std::string_view channel);
void	build_ERR_CHANOPRIVSNEEDED(t_IRC_Client &client,              // 482
            const std::string_view channel);


void	append_common_reply_prefix(std::string &buffer,
			const std::string_view code, const std::string_view nick);

#endif
