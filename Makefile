NAME	= ircserv
CC		= c++
CFLAGS	= -Wall -Wextra -Werror -Wshadow -std=c++20 -MMD -MP -fdiagnostics-color=always
SRC		= src/main.cpp src/server.cpp src/listener.cpp src/client.cpp \
		  src/error.cpp src/msg_parser.cpp src/sender.cpp src/numerics.cpp \
		  src/parsing_utils.cpp src/registration.cpp src/channel_utils.cpp \
		  src/channel_msgs.cpp src/channel_mode.cpp src/channel_privmsg.cpp \
		  src/channel_join.cpp src/channel_part.cpp src/channel_kick.cpp \
		  src/channel_topic.cpp src/channel_names.cpp src/channel_list.cpp \
		  src/channel_invite.cpp src/message_logger.cpp src/pingpong.cpp \
		  src/command_dispatch.cpp src/pass_cmd.cpp src/user_cmd.cpp \
		  src/nick_cmd.cpp src/quit_cmd.cpp src/cap_cmd.cpp
OBJS	= $(SRC:.cpp=.o)
DEPS	= $(SRC:.cpp=.d)

ifeq ($(shell uname),Linux)
SANS = -fsanitize=address,undefined,leak
else
SANS = -fsanitize=address,undefined
endif

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean
	$(MAKE) CFLAGS="$(CFLAGS)" all

fsan: CFLAGS += $(SANS)
fsan: re

-include $(DEPS)

.PHONY: all clean fclean re
