#pragma once

#include <string>

namespace utils {

bool writeFile(const std::string &content, const char *path);

bool isDir(const std::string &p);

const char *validateDirectoryPath(const char *path);

std::string getFileExtension(const std::string &fpath);

} // namespace utils
