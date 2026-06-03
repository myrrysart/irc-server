#ifndef NUMERICS_HPP
# define NUMERICS_HPP

/* Forward declarations */
struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

/* Numeric Replies */
void	send_ERR_NEEDMOREPARAMS(const t_IRC_Client &client); // 461
void	send_ERR_ALREADYREGISTERED(const t_IRC_Client &client); // 462
void	send_ERR_PASSWDMISMATCH(const t_IRC_Client &client); // 464

#endif
