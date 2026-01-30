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

    enum MoveStatus {
        MOVE_SUCCESS,      //!< File moved successfully (atomic or copy)
        MOVE_IO_ERR,       //!< Disk Full, Permission on dest, or other IO error
        MOVE_SYS_ERR,      //!< Permission on src, rename() failed for system reasons
        MOVE_INVALID_STATE //!< File is not open or already moved
    };

    /**
     * @brief Moves the temporary file to a new permanent location.
     *
     * Attempts an atomic rename(). If that fails (e.g. cross-device move), it
     * falls back to a manual copy followed by unlinking the source.
     *
     * @note IMPORTANT: On success, this TempFile object is invalidated (FD closed,
     * path cleared). Ownership of the physical file is transferred to @p destPath.
     * The file will NOT be deleted when this object is destroyed.
     *
     * @param destPath Absolute path to the destination file.
     * @return MoveStatus indicating success or the specific type of failure.
     */
    MoveStatus moveTo(const std::string &destPath);

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
