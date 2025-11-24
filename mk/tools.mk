check:
	@cppcheck --enable=all --inconclusive  --error-exitcode=1 --std=c++98 src/ inc/\
		-I inc/ \
		--suppress=missingIncludeSystem --suppress=unusedFunction \
		--suppress=useStlAlgorithm \
		--suppress=unusedPrivateFunction \
		--suppress=constParameterPointer \
		--suppress=functionConst \
		--suppress=unmatchedSuppression \
		--suppress=normalCheckLevelMaxBranches \
		--suppress=checkersReport
# useStlAlgorithm: std::any_of requires C++11, we use C++98
# unusedPrivateFunction: some functions reserved for future implementation
# constParameterPointer: false positives when pointer is deleted indirectly
# functionConst: inconclusive warnings, false positives for stub functions

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
