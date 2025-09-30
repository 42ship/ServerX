check:
	cppcheck --enable=all --inconclusive --suppress=missingIncludeSystem --suppress=missingInclude --suppress=unusedFunction --std=c++98 src/ inc/

cdb compiledb:
	@compiledb make -n $(NAME) build_tests > /dev/null 2>&1

v valgrind: | $(LOGDIR)
	@$(VALGRIND) ./$(NAME) $(ARGS)
	@find $(LOGDIR) -name 'valgrind-*.log' -type f -empty -delete

$(LOGDIR):
	@mkdir -p $(LOGDIR)

i init:
	@git submodule update --init --remote --recursive

doxy:
	@test -f doxy/Doxyfile || (doxygen -g doxy/Doxyfile && cat doxy/Doxyfile.template > doxy/Doxyfile)
	@doxygen doxy/Doxyfile

compose:
	@docker compose up --build

.PHONY: check cdb compiledb v valgrind i init doxy compose
