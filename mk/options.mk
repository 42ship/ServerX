MAKE		=	@make --no-print-directory
VALGRIND	=	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 --quiet

JOBS := $(filter -j%,$(MAKEFLAGS))

ifeq ($(JOBS),)
    ifeq ($(shell uname -s), Darwin)
        JOBS = -j$(shell sysctl -n hw.ncpu)
    else
        JOBS = -j$(shell nproc)
    endif
endif

FMAKE	=	$(MAKE) $(JOBS)
