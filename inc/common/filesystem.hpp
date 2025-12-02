#pragma once

#include "http/HttpStatus.hpp"
#include <string>

namespace utils {

/**
 * @brief Writes data from an existing file descriptor to a new file on disk.
 *
 * This function reads data from the input file descriptor @p fd using a fixed-size
 * buffer and writes it to a newly created file specified by @p path. If the target
 * file already exists, the function aborts without modifying it.
 *
 * Error handling:
 *  - If the output file already exists, returns 1.
 *  - If creating or writing to the output file fails, the partially written file
 *    is removed (unlink) and the function returns 2.
 *  - If reading from @p fd fails (except EINTR), the partially written file is
 *    removed and the function returns 2.
 *
 * On success, the function returns 0.
 *
 * @param fd   File descriptor to read from.
 * @param path Path to the file that will be created and written to.
 *
 * @return int
 *         - 0 on success
 *         - 1 if the target file already exists
 *         - 2 on read/write/create error (partial file is removed)
 *         - 3 if the input file descriptor is invalid
 */

int writeFile(const int fd, const char *path);

bool writeFile(const std::string &content, const char *path);

bool isDir(const std::string &p);

const char *validateDirectoryPath(const char *path);

std::string getFileExtension(const std::string &fpath);

http::HttpStatus checkFileAccess(const std::string &path, int modeMask,
                                 bool allowDirectory = false);

std::string joinPaths(const std::string &p1, const std::string &p2);

class TempFile {
public:
    TempFile();
    ~TempFile();

    bool open();
    void close();
    operator int() const;
    int fd() const;
    std::string const &path() const;
    bool isOpen() const;

private:
    int fd_;
    std::string filePath_;

    TempFile(TempFile const &TempFile);
    TempFile &operator=(TempFile const &rhs);
};

} // namespace utils
