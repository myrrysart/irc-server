#ifndef IRC_FATSTRUCT_HPP
# define IRC_FATSTRUCT_HPP

#include <netinet/in.h>
# include <string>
# include <poll.h>
# include <unordered_map>
# include <vector>
# include <new> // for hardware_constructive_interference_size

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
# define IN_SETUP 0
# define SERVER_RUNNING BIT(1)
# define SERVER_ERROR BIT(2)

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
// static_assert(sizeof(t_IRC_ChannelMembership) <= 1*CACHE_LINE_SIZE,"IRC_ChannelMembership did not use 1 cache line" );

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
// static_assert(sizeof(t_IRC_Channel) <= 2*CACHE_LINE_SIZE," t_IRC_Channel did not use 2 cache line" );
// IRC_Client state bitmask definitions
//NOTE: state is essentially an error code catcher for the IRC_Client. BIT(0) means client is in and chatting away. Anything else is an active state that needs to be resolved in some way.
# define	IS_OK BIT(0)
typedef struct	s_IRC_Client
{
	t_bmask				state;
	struct sockaddr_in	addr;  //all the adress data. We'll trim it down as needed.
	int					fd;
	std::string			nick;
	std::string			username;
	std::string			realname;
	std::string			hostname;
	std::string			received_message_buffer;
	t_IRC_Channel*		joined_channels;
	int					joined_count;
}						t_IRC_Client;
// static_assert(sizeof(t_IRC_Client) <= 4*CACHE_LINE_SIZE," t_IRC_Client did not use 3 cache line" );

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
