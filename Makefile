NAME = proxy

SRC = $(wildcard src/*.cpp)

HEADER = $(wildcard src/*.hpp)

COUNT_FILES = $(words $(SRC))

FLAG = -Wall -Werror -Wextra

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

OBJ_DIR = obj

SRC_DIR = src

MKDIR = $(sort $(dir $(OBJ)))

CC = g++ -g

all: $(NAME)

$(NAME): $(MKDIR) $(OBJ) $(HEADER)
	@$(CC) $(FLAG) $(OBJ) -o $(NAME)
	@printf "$(ERASE)\r"
	@echo "$(RED)>>$(RESET) $(COUNT_FILES) files $(GREEN)compiled$(RESET)"
	@echo "Type './$(NAME) --help' for more infomation"

$(MKDIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp $(HEADER)
	@$(CC) $(FLAG) -c $< -o $@
	@printf "$(ERASE)$(RED)>> $(YELLOW)[$@]$(GREEN)$(RESET)\r"

clean: 
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)>>$(RESET) $(OBJ_DIR) $(RED)deleted$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@rm -rf logs
	@echo "$(RED)>>$(RESET) $(NAME) $(RED)deleted$(RESET)"

re: fclean all

run: 
	@./$(NAME)

rerun: re 
	@./$(NAME)

ip: #macOS
	@ipconfig getifaddr en0

iplx: #linux
	@hostname -I

RED =		\033[31m
GREEN =		\033[32m
YELLOW =	\033[33m
BLUE =		\033[34m
MAGENTA =	\033[35m
CYAN =		\033[36m
GRAY =		\033[37m
RESET =		\033[0m
ERASE = 	\33[2K

.PHONY: clean fclean re run rerun