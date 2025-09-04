#define _POSIX_C_SOURCE 200809L

#include "http/MimeTypes.hpp"

#include <sstream>
#include <fstream>

MimeTypes::MimeTypes(const std::string &path) : filePath_(path) {
    mimeTypes_ = std::map<std::string, std::string>();
    rtime_.tv_nsec = 0;
    rtime_.tv_sec = 0;
    reload();
    if (mimeTypes_.empty()) {
        throw std::runtime_error("Failed to load MIME types from file: " + filePath_);
    }
}

MimeTypes::MimeTypes(const MimeTypes &other) {
    mimeTypes_ = other.mimeTypes_;
    filePath_ = other.filePath_;
    rtime_ = other.rtime_;
}

MimeTypes &MimeTypes::operator=(const MimeTypes &other) {
    if (this != &other) {
        mimeTypes_ = other.mimeTypes_;
        filePath_ = other.filePath_;
        rtime_ = other.rtime_;
    }
    return *this;
}

const std::string MimeTypes::getMimeType(const std::string &extension) {
    reload();
    if (mimeTypes_.size() == 0) {
        return "text/plain";
    }
    std::map<std::string, std::string>::const_iterator it = mimeTypes_.find(extension);
    if (it != mimeTypes_.end()) {
        return it->second;
    }
    return "text/plain";
}

bool MimeTypes::wasChanged() {
    struct stat fileStat;
    if (stat(filePath_.c_str(), &fileStat) == 0) {
        if (fileStat.st_mtim.tv_nsec != rtime_.tv_nsec ||
            fileStat.st_mtim.tv_sec != rtime_.tv_sec) {
            rtime_ = fileStat.st_mtim;
            return true;
        }
    }
    return false;
}

void MimeTypes::reload() {
    if (!wasChanged()) {
        return;
    }
    std::ifstream file(filePath_.c_str());
    std::string fileText;
    // Read the MIME types file line by line
    // and check if the extension matches any of the defined types
    if (file) {
        mimeTypes_.clear();
        while (std::getline(file, fileText)) {
            // skip comments
            if (fileText.find_first_of('#') == 0) {
                continue;
            }

            std::string mimeType = fileText.substr(0, findFirstSpace(fileText));
            std::string extensions = fileText.substr(findFirstNonSpace(fileText, mimeType.size()));
            std::istringstream iss(extensions);
            std::string ext;
            while (iss >> ext) {
                mimeTypes_[ext] = mimeType;
            }
        }
    }
    file.close();
}

size_t MimeTypes::findFirstSpace(const std::string str) {
    size_t pos = 0;
    size_t size = str.size();
    const char *tmp = str.c_str();
    while (pos < size) {
        if (tmp[pos] == ' ' || tmp[pos] == '\t' || tmp[pos] == '\v') {
            return pos;
        }
        pos++;
    }
    return pos;
}

size_t MimeTypes::findFirstNonSpace(const std::string str, size_t startPos) {
    size_t pos = startPos;
    size_t size = str.size();
    const char *tmp = str.c_str();
    while (pos < size) {
        if (tmp[pos] != ' ' && tmp[pos] != '\t' && tmp[pos] != '\v') {
            return pos;
        }
        pos++;
    }
    return pos;
}

MimeTypes::~MimeTypes() {
}