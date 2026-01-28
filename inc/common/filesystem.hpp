#pragma once

#include "http/HttpStatus.hpp"
#include <ctime>
#include <string>
#include <sys/stat.h>
#include <vector>

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

private:
    int fd_;
    std::string filePath_;

    TempFile(TempFile const &TempFile);
    TempFile &operator=(TempFile const &rhs);
};

struct FileEntry {
    std::string name;
    bool isDir;
    time_t mtime;
    off_t size;
};

/**
 * @brief Reads a directory, stats all files, and returns them sorted by name.
 * Uses only opendir/readdir/closedir/stat.
 */
bool getDirectoryEntries(std::string const &path, std::vector<FileEntry> &entries);

/**
 * @brief Simple wrapper for stat to check existence and get details.
 */
bool getFileStatus(std::string const &path, struct stat &buf);

} // namespace utils
