# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: schongte <schongte@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/18 09:56:39 by schongte          #+#    #+#              #
#    Updated: 2025/02/08 15:08:19 by schongte         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Binary:
NAME = webserv
TEST_NAME = unit_tests

# Directories:
SRC_DIR = src
OBJ_DIR = objects
CORE_DIR = $(SRC_DIR)/core
PLUGINS_DIR = $(SRC_DIR)/plugins
TEST_DIR = tests
CONFIG_DIR = config
INCLUDE_DIR = include

# Source files:
CORE_SRC = main.cpp \
          Parser.cpp \
          HttpRequest.cpp \
          HttpResponse.cpp \
          Router.cpp \
          FileHandler.cpp \
          ConnectionManager.cpp \
          Connection.cpp \
          Server.cpp \
          PluginManager.cpp 

PLUGIN_SRC = plugin.cpp

SRC = $(addprefix $(CORE_DIR)/, $(CORE_SRC)) \
      $(addprefix $(PLUGINS_DIR)/, $(PLUGIN_SRC))

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

TEST_SRC = unit_tests.cpp
TEST_OBJ = $(TEST_SRC:%.cpp=$(OBJ_DIR)/%.o)

# Colors:
GREEN		=	\e[38;5;118m
YELLOW		=	\e[38;5;226m
RED			=	\e[38;5;196m
RESET		=	\e[0m
_SUCCESS	=	[$(GREEN)SUCCESS$(RESET)]
_INFO		=	[$(YELLOW)INFO$(RESET)]
_ERROR		=	[$(RED)ERROR$(RESET)]

# Compiler flags:
CXX = g++
CXXFLAGS = -Wall  -Werror -std=c++98
DEBUG_FLAGS = -g -DDEBUG
SANITIZE_FLAGS = -fsanitize=address -fsanitize=undefined
INC = -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(CORE_DIR) -I$(PLUGINS_DIR)

# Default config path
DEFAULT_CONFIG = $(CONFIG_DIR)/default.conf

# Targets:
all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(_INFO) Compiling $(NAME)"
	@$(CXX) $(CXXFLAGS) -o $(NAME) $^
	@echo "$(_SUCCESS) $(NAME) compiled"

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean $(NAME)
	@echo "$(_SUCCESS) Debug version compiled"

sanitize: CXXFLAGS += $(SANITIZE_FLAGS) $(DEBUG_FLAGS)
sanitize: clean $(NAME)
	@echo "$(_SUCCESS) Sanitized version compiled"

$(OBJ_DIR)/core/%.o: $(CORE_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/core
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJ_DIR)/plugins/%.o: $(PLUGINS_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)/plugins
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(TEST_NAME): CXXFLAGS += $(DEBUG_FLAGS)
$(TEST_NAME): $(filter-out $(OBJ_DIR)/core/main.o, $(OBJ)) $(TEST_OBJ)
	@echo "$(_INFO) Compiling unit tests"
	@$(CXX) $(CXXFLAGS) $^ -o $(TEST_NAME)
	@echo "$(_SUCCESS) Unit tests compiled"

test: $(TEST_NAME)
	@echo "$(_INFO) Running unit tests"
	@./$(TEST_NAME)
	@echo "$(_SUCCESS) All tests passed"

stress: $(NAME)
	@echo "$(_INFO) Running stress tests"
	@chmod +x tests/stress_test.sh
	@./tests/stress_test.sh
	@echo "$(_SUCCESS) Stress tests completed"

config_check: $(NAME)
	@echo "$(_INFO) Validating configuration files"
	@./$(NAME) -t $(DEFAULT_CONFIG)
	@echo "$(_SUCCESS) Configuration validation passed"

clean:
	@rm -rf $(OBJ_DIR)
	@rm -rf $(TEST_DIR)/stress_logs
	@echo "$(_INFO) Objects cleaned"

fclean: clean
	@rm -f $(NAME) $(TEST_NAME)
	@echo "$(_SUCCESS) $(NAME) and tests cleaned"

re: fclean all

run: $(NAME)
	@echo "$(_INFO) Starting server with default configuration"
	@./$(NAME) $(DEFAULT_CONFIG)

valgrind: $(NAME)
	@echo "$(_INFO) Running with valgrind"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) $(DEFAULT_CONFIG)

.PHONY: all clean fclean re test debug sanitize stress config_check run valgrind
