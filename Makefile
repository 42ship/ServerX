NAME		=	webserv
ARGS		?=	config/example.conf
# =============================== DIRECTORIES ================================ #
HDIR		=	inc
ODIR		=	obj
SDIR		=	src
LOGDIR		=	logs
# =============================== COMPILATION ================================ #
CXX			=	c++
CXXFLAGS	=	-std=c++98 -Wall -Wextra -Werror -MMD -MP
CXXFLAGS	+=	-I$(HDIR) -I$(HDIR)/config
# ================================== FILES =================================== #
SRCS		:=	$(shell find $(SDIR) -name "*.cpp")
OBJS		:=	$(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRCS))
DEPS		:=	$(patsubst %.o,%.d,$(OBJS))
DIRS		=	$(sort $(dir $(OBJS)))
# ================================= INCLUDES ================================= #
include mk/tools.mk
include mk/options.mk
include mk/help.mk
include mk/tests.mk
# ================================== RULES =================================== #
all:
	@$(FMAKE) $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJS): $(ODIR)/%.o: $(SDIR)/%.cpp | $(DIRS)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(DIRS):
	@mkdir -p $@

r run:
	@$(MAKE) all > /dev/null
	@./$(NAME) $(ARGS)

rr rerun: clean r

c clean:
	@rm -rf $(ODIR)

f fclean: clean test_clean
	@rm -rf $(NAME)

re: fclean
	@$(MAKE) all

debug: clean
	@$(MAKE) -j1 all 2>&1 | while read line; do \
	echo "$$line"; \
	done

-include $(DEPS)
-include $(TEST_DEPS)

.PHONY: all build c clean f fclean re r run rr rerun
