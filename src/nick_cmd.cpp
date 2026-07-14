
#include "../lib/irc_fatstruct.hpp"
#include "../lib/numerics.hpp"
#include "../lib/parser.hpp"
#include "../lib/commands.hpp"
#include "../lib/channel.hpp"

#include <cctype> // for std::isdigit()
#include <string_view>
#include <unordered_map>

static bool	is_nickname_valid(std::string_view nickname);
static void	prepare_to_store_new_nick_and_alert_clients(t_IRC_Client &client,
	            std::string_view new_nick);
static void	store_new_nickname(t_IRC_Client &client, std::string_view new_nick);
static bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
	            int fd, std::string_view new_nick);

void	execute_NICK_cmd(t_IRC_Client &client, t_IRC_Server &server)
{
	// no nickname / empty nickname provided
	if (!client.parser.n_params || client.parser.params[0].empty())
	{
		build_ERR_NONICKNAMEGIVEN(client);
		return;
	}

	std::string_view	new_nick{client.parser.params[0]};
	trim_nickname_if_longer_than_max_nicklen(new_nick);

	// validation: check that requested nickname contains only valid characters
	// NOTE: It is important that this step is done here, and not later on.
	// Upon setup of a new client, the default nickname is set to '*', which is
	// an invalid placeholder - on purpose.
	// A client requesting the nickname '*' should be refused, and such a nickname
	// should not be compared against any other default clients' nickname, when
	// this function checks whether it is already taken...
	if (!is_nickname_valid(new_nick))
	{
		build_ERR_ERRONEOUSNICKNAME(client, new_nick);
		return;
	}

	// Silently ignore request for an already identical nickname (same client).
	// However, if the equality is only 'case-insensitive', change the nickname,
	// alert the client and fellow channel members
	if (is_flag_set(client.state, t_IRC_Client::NICK)
			&& are_equal_strs_case_insensitive(new_nick, client.nick))
	{
		if (new_nick != client.nick)
			prepare_to_store_new_nick_and_alert_clients(client, new_nick);
		return;
	}

	// handle an already taken nickname
	if (is_nick_already_in_use(server.clients, client.fd, new_nick))
	{
		build_ERR_NICKNAMEINUSE(client, new_nick);
		return;
	}

	prepare_to_store_new_nick_and_alert_clients(client, new_nick);
}

static void	prepare_to_store_new_nick_and_alert_clients(t_IRC_Client &client,
	            std::string_view new_nick)
{
	// nickname request is valid: update client's state
	client.state |= t_IRC_Client::NICK;

	// Only store new nickname if client is:
	//  • already registered
	//  • unregistered, but has provided the password first
	if (is_flag_set(client.state, t_IRC_Client::REGISTERED))
	{
		// temporarily store previous nick in order to integrate it to the reply
		std::string	old_nick{client.nick};

		// Store new nickname (has to be a deep copy)
		store_new_nickname(client, new_nick);

		// Broadcast NICK reply to the requesting client and to all fellow
		// channelers (but not more than once per client)
		broadcast_nick_change(client, old_nick);

	}
	else if (is_flag_set(client.state, t_IRC_Client::PSWD_FIRST))
	{
		// Client registration process seems to be going well:
		// Store new nickname (has to be a deep copy)
		store_new_nickname(client, new_nick);
	}
	/* 'else': we return, silently ignoring the NICK change. No need to inform
	* anyone about it since the client is not yet connected and no one else is
	* aware of its connection attempt - and it will be disconnected soon enough,
	* since their registration process has failed */
}

static void	store_new_nickname(t_IRC_Client &client, std::string_view new_nick)
{
	for (size_t	i = 0; i < new_nick.size(); ++i)
		client.nick_buf[i] = new_nick[i];
	client.nick = std::string_view{client.nick_buf, new_nick.size()};
}

static bool	is_nickname_valid(std::string_view nickname)
{
	size_t	len = nickname.size();

	// refuse leading: digit, '#', ':' or '&'
	if (len)
	{
		char	c = nickname[0];
		if (std::isdigit(static_cast<unsigned char>(c))
				|| c == '#' || c == ':' || c == '&')
			return false;
	}

	if (nickname.find_first_not_of(t_IRC_Client::nick_whitelist) != std::string_view::npos)
		return false;
	return true;
}

// uses 'ascii' CASEMAPPING, meaning case-insensitive checks: "tommy" == "ToMMY"
static bool	is_nick_already_in_use(const std::unordered_map<int, t_IRC_Client> &clients,
	            int fd, std::string_view new_nick)
{
	// iterator->first is the fd key, iterator->second is t_IRC_Client value
	for (std::unordered_map<int, t_IRC_Client>::const_iterator iterator = clients.begin();
		iterator != clients.end();
		++iterator)
	{
		// avoid comparing newly requested nickname with requesting client's current nickname
		if (iterator->first == fd)
			continue;

		if (are_equal_strs_case_insensitive(new_nick, iterator->second.nick))
			return true;
	}
	return false;
}
