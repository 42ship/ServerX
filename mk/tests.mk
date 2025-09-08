DOCTEST_DIR		=	lib/doctest/doctest
TEST_NAME		=	run_tests
TEST_DIR		=	tests
TEST_SRCS		:=	$(shell find $(TEST_DIR) -name "*.cpp")
TEST_OBJS		:=	$(patsubst $(TEST_DIR)/%.cpp,$(ODIR)/tests/%.o,$(TEST_SRCS))
TEST_DEPS		:=	$(patsubst %.o,%.d,$(TEST_OBJS))

TEST_CXXFLAGS	=	$(filter-out -Werror, $(CXXFLAGS)) -I$(DOCTEST_DIR) -std=c++17

PROJECT_OBJS_FOR_TESTS	:=	$(filter-out $(ODIR)/main.o, $(OBJS))

DIRS	+=	$(sort $(dir $(TEST_OBJS)))

t test: | $(DOCTEST_DIR)
	@$(FMAKE) build_tests > /dev/null
	@./$(TEST_NAME) -ni

build_tests: $(TEST_NAME)

vt valgrind_test: | $(DOCTEST_DIR) $(LOGDIR)
	@$(FMAKE) build_tests > /dev/null
	@$(VALGRIND) ./$(TEST_NAME) -ni
	@find $(LOGDIR) -name 'valgrind-*.log' -type f -empty -delete

$(TEST_NAME): $(TEST_OBJS) $(PROJECT_OBJS_FOR_TESTS)
	@$(CXX) $(TEST_CXXFLAGS) -o $@ $^

$(ODIR)/tests/%.o: $(TEST_DIR)/%.cpp | $(DIRS)
	@echo "🔨 Compiling Test $<"
	@$(CXX) $(TEST_CXXFLAGS) -c -o $@ $<

test_clean:
	@rm -rf $(TEST_NAME)

$(DOCTEST_DIR):
	@$(MAKE) init

.PHONY: t test build_tests test_clean vt valgrind_test
