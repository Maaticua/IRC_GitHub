NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

SRCS		= main.cpp Server.cpp
OBJS		= $(SRCS:.cpp=.o)

# Couleurs pour le terminal
RED			= \033[31m
YELLOW		= \033[33m
GREEN		= \033[0;32m
RESET		= \033[0m


all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN)$(NAME) compilé avec succès !$(RESET)\n"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@printf "$(YELLOW)Fichiers objets supprimés.$(RESET)\n"

fclean: clean
	@rm -f $(NAME)
	@printf "$(RED)Exécutable $(NAME) supprimé.$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re