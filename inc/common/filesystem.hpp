#pragma once

#include "http/HttpStatus.hpp"
#include <string>

namespace utils {

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

    /**
     * @brief Moves a file atomically if possible, or copies it as a fallback.
     *
     * Attempts to move a file from @p srcPath to @p destPath using the POSIX
     * rename() system call, which performs an atomic move when both paths
     * reside on the same filesystem.
     *
     * If rename() fails (e.g. due to EXDEV when crossing filesystem boundaries,
     * or insufficient permissions), the function falls back to copying the file
     * contents in binary mode using buffered I/O.
     *
     * The destination file is created or truncated as needed. On any write or
     * read failure, the partially written file is removed to ensure filesystem
     * consistency.
     *
     * This function is suitable for server-side use cases such as safe file uploads,
     * CGI script output, and temporary file management.
     *
     * @param srcPath   Absolute path to the source file (must exist and be readable).
     * @param destPath  Absolute path to the destination file.
     *
     * @return int Status code:
     *         - 0 → success (file moved or copied)
     *         - 1 → copy or write failure (partial file removed)
     *         - 2 → invalid source file, access denied, or unrecoverable error
     *
     * @note This function does not remove the source file (@p srcPath).
     *       Caller is responsible for cleanup after successful or failed operation.
     */
    static int moveOrCopyFile(const std::string &srcPath, const std::string &destPath);

private:
    int fd_;
    std::string filePath_;

    TempFile(TempFile const &TempFile);
    TempFile &operator=(TempFile const &rhs);
};

} // namespace utils
