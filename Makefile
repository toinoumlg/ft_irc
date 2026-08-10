NAME        = ircserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -g -O0
DEPFLAGS    = -MMD -MP
SOURCE_DIR  = source
OBJECTS_DIR = build
INCLUDE_DIR = include

CPP_FILES = main Client Irc Channel ServerResponse StatusCode
SOURCES   = $(addprefix $(SOURCE_DIR)/, $(addsuffix .cpp, $(CPP_FILES)))
OBJECTS   = $(patsubst $(SOURCE_DIR)/%.cpp, $(OBJECTS_DIR)/%.o, $(SOURCES))
DEPS      = $(OBJECTS:.o=.d)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

$(OBJECTS_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJECTS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re