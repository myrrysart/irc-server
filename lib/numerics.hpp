#ifndef NUMERICS_HPP
# define NUMERICS_HPP

# include <string_view>

/* Forward declarations */
struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

// WARN: All of these are just placeholders for now and have to be re-written.
/* Numeric Replies */
void	send_ERR_NONICKNAMEGIVEN(const t_IRC_Client &client);    // 431
void	send_ERR_ERONEOUSNICKNAME(const t_IRC_Client &client,    // 432
            const std::string_view &new_nick);
void	send_ERR_NICKNAMEINUSE(const t_IRC_Client &client,       // 433
            const std::string_view &new_nick);
void	send_ERR_NEEDMOREPARAMS(const t_IRC_Client &client);     // 461
void	send_ERR_ALREADYREGISTERED(const t_IRC_Client &client);  // 462
void	send_ERR_PASSWDMISMATCH(const t_IRC_Client &client);     // 464

#endif
