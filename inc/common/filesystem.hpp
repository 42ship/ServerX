#pragma once

#include <string>

namespace utils {

bool writeFile(const std::string &content, const char *path);

bool isDir(const std::string &p);

const char *validateDirectoryPath(const char *path);

std::string getFileExtension(const std::string &fpath);

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
