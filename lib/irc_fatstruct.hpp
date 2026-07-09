#ifndef IRC_FATSTRUCT_HPP
# define IRC_FATSTRUCT_HPP

# include <netinet/in.h>
# include <string>
# include <poll.h>
# include <unordered_map>
# include <unordered_set>
# include <vector>
# include <cstdint> // for fixed width integer data types
# include <new> // for hardware_constructive_interference_size
# include <string_view>

# define MAX_CLIENTS 128
# define MAX_CHANNELS 64
# define MAX_PENDING_CONNECTIONS 32
# define MAX_CHANNELS_PER_CLIENT 32

#ifdef __cpp_lib_hardware_interference_size
# define CACHE_LINE_SIZE std::hardware_constructive_interference_size
#else
# define CACHE_LINE_SIZE 64
#endif

// bitmask helper macro for declaring bit state macros
# define BIT(x) (1u << (x))
// bitmask assumption that if a state of bitmask is 0, it is still in setup phase and have just been created

// bitmask typedefinition for fat struct bitfields.
//NOTE: Create a new one if more bits are needed (ex. typedef unsigned long t_long_bmask;)
typedef unsigned int	t_bmask;

// forward declarations
struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

// the states of clients in individual channels
# define IS_OPERATOR BIT(0)

// IRC_Channel mode bitmask definitions
# define	INVITE BIT(0)
# define	TOPIC  BIT(1)
# define	KEY BIT(2)
# define	LIMIT BIT(3)
typedef struct	s_IRC_Channel
{
	t_bmask										mode;
	std::string									name;
	std::string									topic;
	std::string									key;
	size_t										user_limit;
	std::unordered_map<t_IRC_Client*, t_bmask>	members;
	std::unordered_set<t_IRC_Client*>			invited;
}												t_IRC_Channel;
static_assert(sizeof(t_IRC_Channel) <= 42*CACHE_LINE_SIZE," t_IRC_Channel did not use 42 cache line" );

typedef struct	s_parser
{
	static constexpr std::string_view	commands[] = {
		"PASS",
		"NICK",
		"USER",
		"QUIT",
		"JOIN",
		"PART",
		"PRIVMSG",
		"MODE",
		"KICK",
		"INVITE",
		"TOPIC",
		"NAMES",
		"LIST",
		"PING",
		"PONG"
	};
	// NOTE: do not implement OPER: we need channel operators, not IRC operators.
	// WARN: do we need to implement CAP? Is that what allows a user to become operator?

	static constexpr size_t		n_valid_cmds = sizeof(commands) / sizeof(std::string_view);

	/* NOTE: max_params is currently set to 255, because the longest message the
	* server accepts is 512 bytes long, the last of which is either '\n' or "\r\n".
	* Even in an improbable scenario where the message's command and each of the
	* following parameters would be only 1 byte long (separated by a space),
	* we could have as many as 254-255 arguments. */
	static constexpr size_t		buf_size = 512;
	static constexpr size_t		max_params = 255;

	// Computes the longest available command's length at compile time, so that
	// future changes in the command list would adjust this value automatically.
	static constexpr size_t		longest_cmd_size = [] {
		size_t len = 0;
		for (size_t i = 0; i < n_valid_cmds; ++i)
		{
			if (len < commands[i].size())
				len = commands[i].size();
		}
		return len;
	}();

	size_t				n_params; // the 'trailing' parameter is not split into differnet fields, and counts as 1
	std::string_view	verb;
	std::string_view	params[max_params];
	static char			verb_in_caps[longest_cmd_size]; // lives outside of the struct and shared between clients. WARN: If threads are introduced in this project, this will not be thread safe!

}	t_parser;
static_assert(sizeof(t_parser) <= 65*CACHE_LINE_SIZE, "t_parser did not use 65 cache lines");
// WARN: this struct is quite large - is there a way to reduce it?

//NOTE: state is essentially an error code catcher for the IRC_Client. BIT(0) means client is in error state (or has asked to quit) and should be disconnected. Anything else is an active state that needs to be resolved in some way.
typedef struct	s_IRC_Client
{
	// IRC_Client state bitmask definitions
	enum {
		DISCONNECT   = BIT(0),
		REGISTERED   = BIT(1),
		PSWD_FIRST   = BIT(2),
		PSWD_CORRECT = BIT(3),
		NICK         = BIT(4),
		USERNAME     = BIT(5),
		DISCARD_MSG  = BIT(6)
	};

	// IRC protocol's username length parameter
	static constexpr size_t	userlen = 10;

	// "If <nickname> is longer than the server allows (...), it is silently truncated"
	static constexpr size_t	max_nicklen = 30;

	// Allows reducing calls to std::string.erase() for the output buffer, since
	// erasing string's beginning may require moving its tail to the front.
	static constexpr size_t	offset_threshold = 8192;

	// whitelist of allowed characters for clients' nickname
	// Updating the allowed symbols in this array would carry over whole program.
	static constexpr const std::string_view	nick_whitelist = {
		"[]{}\\|#&:$%<>_-" // allowed symbols: can be modified safely.
		"0123456789" // digits
		"abcdefghijklmnopqrstuvwxyz" // lowercase alphabet
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" // uppercase alphabet
	};

	// string_view into nick_whitelist's special symbol characters, allows to
	// send those allowed symbols to clients, if they choose an erroneous nickname
	static constexpr std::string_view	allowed_symbols_nick = [] {
		size_t	i = 0;

		while (i < nick_whitelist.size() &&
				(nick_whitelist[i] < '0' || nick_whitelist[i] > '9'))
			++i;

		return (std::string_view{nick_whitelist.data(), i});
	}();

	t_bmask								state;
	struct sockaddr_in					addr;  //NOTE: Is this needed here?
	int									fd;
	std::string_view					nick;
	char								nick_buf[max_nicklen]; // not nullterminated, use 'nick' instead
	std::string							username;
	std::string							realname;
	char								hostname[INET_ADDRSTRLEN]; // This array is null terminated when initialized by inet_ntop(). Change macro to 'INET6_ADDRSTRLEN' if server ever switches to TCP6 ('AF_INET6').
	std::string							received_message_buffer;
	std::string							send_message_buffer;
	size_t								send_offset;
	t_parser							parser;
	std::unordered_set<t_IRC_Channel*>	joined_channels;
}										t_IRC_Client;
static_assert(sizeof(t_IRC_Client) <= 128*CACHE_LINE_SIZE," t_IRC_Client did not use 128 cache line" );

// IRC_Server state bitmask definitions
typedef struct	s_IRC_Server
{
	enum {
		FATAL_ERROR = BIT(0)
	};

	t_bmask											state;
	static constexpr const char						name[] = "humble_server";
	static constexpr int							poll_timeout = 1000;
	static constexpr const char						version[] = "0.042"; // remember to update when upgrading ;-)
	int												listen_fd;
	uint16_t										port;
	std::string_view								password;
	std::unordered_map<int, t_IRC_Client>			clients;
	std::unordered_map<std::string, t_IRC_Channel>	channels;
	std::vector<pollfd>								poll_fds;

	/* destructor */
	~s_IRC_Server();

}													t_IRC_Server;

/* Bit mask utils */
bool	is_flag_set(const t_bmask state, const unsigned int mask);

/* Error logging */
void	log_error(const char *error, const char *context, const char *filename,
            const int line_num);
void	set_fatal_error_flag_and_log(t_bmask &state, const char *context,
            const char *filename, const int line_num);

#endif//IRC_FATSTRUCT_HPP
