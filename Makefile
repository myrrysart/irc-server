NAME	= ircserv
CC		= c++
CFLAGS	= -Wall -Wextra -Werror -std=c++98 -MMD -MP
SRC		= src/main.cpp
OBJS	= $(SRC:.cpp=.o)
DEPS	= $(SRC:.cpp=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
