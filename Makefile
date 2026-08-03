NAME = webserv
SRCS = srcs/main.cpp \
	srcs/server/Server.cpp \
	srcs/server/Socket.cpp \
	srcs/server/Client.cpp \
	srcs/config/Config.cpp \
	srcs/config/ServerConfig.cpp \
	srcs/config/LocationConfig.cpp \
	srcs/HttpRequest.cpp \
	srcs/HttpResponse.cpp \
	srcs/handlers/StaticHandler.cpp \
	srcs/handlers/CgiHandler.cpp \


OBJDIR = obj
OBJS = $(SRCS:srcs/%.cpp=$(OBJDIR)/%.o)
CC = c++
CFLAGS = -fsanitize=address -g3 -std=c++98 -Wall -Wextra -Werror
RM = rm -rf

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o : srcs/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	$(RM) $(OBJDIR)

fclean : clean
	$(RM) $(NAME)

re : fclean all

.PHONY: all clean fclean re