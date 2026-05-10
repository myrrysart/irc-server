#ifndef IRC_FATSTRUCT_HPP
# define IRC_FATSTRUCT_HPP

# include <string>

# define MAX_CLIENTS 128
# define MAX_CHANNELS 64

// bitmask helper macro for declaring bit state macros
# define BIT(x) (1u << (x))
// bitmask assumption that if a state of bitmask is 0, it is still in setup phase and have just been created
# define IN_SETUP 0
// bitmask typedefinition for fat struct bitfields. 
//NOTE: Create a new one if more bits are needed (ex. typedef unsigned long t_long_bmask;)
typedef unsigned int	t_bmask;

// forward declarations
struct	s_IRC_Client;
typedef s_IRC_Client t_IRC_Client;

// the states of clients in individual channels
# define IS_OPERATOR bit(0)
# define IS_BANNED	bit(2)
typedef struct	s_IRC_ChannelMembership 
{
	t_bmask			state;
	t_IRC_Client*	client;
}					t_IRC_ChannelMembership;

// IRC_Channel mode bitmask definitions
# define	INVITE BIT(0)
# define	TOPIC  BIT(1)
# define	KEY BIT(2)
# define	LIMIT BIT(3)
typedef struct	s_IRC_Channel 
{
	t_bmask					state;
	t_bmask					mode;
	std::string				name;
	std::string				topic;
	std::string				key;
	int						user_limit;
	t_IRC_ChannelMembership	members[MAX_CLIENTS];
	int						member_count;
}							t_IRC_Channel;

// IRC_Client state bitmask definitions
//NOTE: state is essentially an error code catcher for the IRC_Client. BIT(0) means client is in and chatting away. Anything else is an active state that needs to be resolved in some way.
# define	IS_OK BIT(0)
typedef struct	s_IRC_Client 
{
	t_bmask			state;
	int				fd;
	std::string		nick;
	std::string		username;
	std::string		realname;
	std::string		hostname;
	std::string		received_message_buffer;
	int				received_message_len;
	t_IRC_Channel*	joined_channels[MAX_CHANNELS];
	int				joined_count;
}					t_IRC_Client;

// IRC_Server state bitmask definitions
//NOTE: state is essentially an error code catcher for the IRC_Server. BIT(0) means server is running smoothly, anything else is an error case.
typedef struct	s_IRC_Server 
{
	t_bmask			state;
	int				listen_fd;
	int				port;
	std::string		password;
	t_IRC_Client	clients[MAX_CLIENTS];
	int				client_count;
	t_IRC_Channel	channels[MAX_CHANNELS];
	int				channel_count;
}					t_IRC_Server;

#endif//IRC_FATSTRUCT_HPP
