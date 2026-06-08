#ifndef IRC_FATSTRUCT_HPP
# define IRC_FATSTRUCT_HPP

# include <netinet/in.h>
# include <string>
# include <poll.h>
# include <unordered_map>
# include <vector>
# include <new> // for hardware_constructive_interference_size
# include <string_view>

# define MAX_CLIENTS 128
# define MAX_CHANNELS 64
# define MAX_PENDING_CONNECTIONS 32

#ifdef __cpp_lib_hardware_interference_size
# define CACHE_LINE_SIZE std::hardware_constructive_interference_size
#else
# define CACHE_LINE_SIZE 64
#endif

// bitmask helper macro for declaring bit state macros
# define BIT(x) (1u << (x))
// bitmask assumption that if a state of bitmask is 0, it is still in setup phase and have just been created
# define SERVER_DOWN 0
# define SERVER_RUNNING BIT(1)

// bitmask typedefinition for fat struct bitfields.
//NOTE: Create a new one if more bits are needed (ex. typedef unsigned long t_long_bmask;)
typedef unsigned int	t_bmask;

// forward declarations
struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

// the states of clients in individual channels
# define IS_OPERATOR BIT(0)
# define IS_BANNED	BIT(2)
typedef struct	s_IRC_ChannelMembership
{
	t_bmask			state;
	t_IRC_Client*	client;
}					t_IRC_ChannelMembership;
static_assert(sizeof(t_IRC_ChannelMembership) <= 1*CACHE_LINE_SIZE,"IRC_ChannelMembership did not use 1 cache line" );

// IRC_Channel state bitmask definitions
# define	IS_RUNNING BIT(0)
// IRC_Channel mode bitmask definitions
# define	INVITE BIT(0)
# define	TOPIC  BIT(1)
# define	KEY BIT(2)
# define	LIMIT BIT(3)
# define	OPERATOR_PRIVILEGE BIT(4)
typedef struct	s_IRC_Channel
{
	t_bmask						state;
	t_bmask						mode;
	std::string					name;
	std::string					topic;
	std::string					key;
	int							user_limit;
	t_IRC_ChannelMembership*	members;
	int							member_count;
}								t_IRC_Channel;
static_assert(sizeof(t_IRC_Channel) <= 2*CACHE_LINE_SIZE," t_IRC_Channel did not use 2 cache line" );

typedef struct	s_parser
{
	/* eventual PREFIX implementations */
	// enum	Flags : uint8_t
	// {
	// 	HAS_TAGS     = BIT(0),
	// 	HAS_SOURCE   = BIT(1),
	// 	HAS_TRAILING = BIT(2)
	// };
	// WARN: is this used?

	/* NOTE: max_params is currently set to 255, because the longest message the
	 * server accepts is 512 bytes long, the last of which is either '\n' or "\r\n".
	* Even in an improbable scenario where the message's command and each of the
	* following parameters would be only 1 byte long (separated by a space),
	* we could have as many as 254-255 arguments. */
	static constexpr size_t		buf_size = 512;
	static constexpr size_t		max_params = 255;

	static constexpr const char	*commands[] = {
		"NICK",
		"PASS", // should align with the password for our server (argv[2])
		"USER",
		"JOIN", // "lets users join a channel"// WARN: is this the exact command needed to be implemented for joining a channel?
		"KICK",
		"INVITE",
		"PART", // WARN: extra but nice to have: "lets users leave a channel."
		"PING",
		"PONG",
		"PRIVMSG" // "used to send private messages between users, as well as to send messages to channels"
	};
	// NOTE: do not implement OPER: we need channel operators, not IRC operators.

	/* eventual PREFIX implementations */
	// t_bmask			state;
	//std::string_view	tags; // eventual tokens
	//std::string_view	source; // eventual tokens
	// WARN: is this used?

	size_t				n_params; // the 'trailing' parameter is not split into differnet fields, and counts as 1
	std::string_view	verb; // WARN: can it ONLY be one single word / 3 digits?
	std::string_view	params[max_params];

}	t_parser;
static_assert(sizeof(t_parser) <= 65*CACHE_LINE_SIZE, "t_parser did not use 65 cache lines");
// WARN: this struct is quite large - is there a way to reduce it?

// IRC_Client state bitmask definitions
//NOTE: state is essentially an error code catcher for the IRC_Client. BIT(0) means client is in and chatting away. Anything else is an active state that needs to be resolved in some way.
typedef struct	s_IRC_Client
{
	enum Flags : uint8_t {
		IS_OK         = BIT(0),
		PASSWORD      = BIT(1),
		NICK          = BIT(2),
		USERNAME      = BIT(3),
		ERROR         = BIT(4),
		DISCARD_MSG   = BIT(5)
	};

	t_bmask				state;
	struct sockaddr_in	addr;  //all the adress data. We'll trim it down as needed.
	int					fd;
	std::string			nick;
	std::string			username;
	std::string			realname;
	std::string			hostname;
	std::string			received_message_buffer;
	std::string			send_buffer;
	t_parser			parser;
	t_IRC_Channel*		joined_channels;
	int					joined_count;
}						t_IRC_Client;
static_assert(sizeof(t_IRC_Client) <= 68*CACHE_LINE_SIZE," t_IRC_Client did not use 68 cache line" );

// IRC_Server state bitmask definitions
//NOTE: state is essentially an error code catcher for the IRC_Server. BIT(0) means server is running smoothly, anything else is an error case.
typedef struct	s_IRC_Server
{
	t_bmask									state;
	int										listen_fd;
	int										port;
	std::string								password;
	std::unordered_map<int, t_IRC_Client>	clients;
	t_IRC_Channel							channels[MAX_CHANNELS];
	int										channel_count;
	std::vector<pollfd>						poll_fds;
}											t_IRC_Server;

#endif//IRC_FATSTRUCT_HPP
