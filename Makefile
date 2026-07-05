NAME	= ircserv
CC		= c++
CFLAGS	= -Wall -Wextra -Werror -Wshadow -std=c++20 -MMD -MP -fdiagnostics-color=always
SRC		= src/main.cpp src/server.cpp src/listener.cpp src/client.cpp \
		  src/error.cpp src/msg_parser.cpp src/sender.cpp src/numerics.cpp \
		  src/parsing_utils.cpp src/commands.cpp src/registration.cpp \
		  src/command_utils.cpp src/channel_utils.cpp src/channel_msgs.cpp \
		  src/channel_mode.cpp src/channel_cmds.cpp
OBJS	= $(SRC:.cpp=.o)
DEPS	= $(SRC:.cpp=.d)
SANS	= -fsanitize=address,undefined

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
