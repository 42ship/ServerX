#pragma once

#include <string>

bool removeDirectoryRecursive(const std::string &path);

void writeFile(const std::string& content, const char* path);

void writeFile(const std::string& content, const char* path, int perm);