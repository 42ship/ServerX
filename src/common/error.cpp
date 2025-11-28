#include "common/error.hpp"
#include <cstring>
#include <unistd.h>

void print_errno_to_fd(int fd, int err_num) {
    char buf[256];

    char *err = strerror_r(err_num, buf, sizeof(buf));
    ssize_t i = write(fd, err, strlen(err));
    (void)i;
}
