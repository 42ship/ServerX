#include "http/ResponseContent.hpp"

#include <fstream>
#include <sstream>

#include "http/MimeTypes.hpp"

static std::string toLower(const std::string &s) {
    std::string r = s;
    for (std::string::size_type i = 0; i < r.size(); ++i)
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    return r;
}

http::ResponseContent::ResponseContent() : body_(""), type_("") {
}

http::ResponseContent::ResponseContent(const char *path, MimeTypes *mime) {
    std::ifstream file(path);
    std::string fileText;
    if (file) {
        while (std::getline(file, fileText)) {
            body_ += fileText;
        }
        setFileType(path, mime);
    }
}

http::ResponseContent::ResponseContent(const std::string &body, const std::string &type)
    : body_(body), type_(type) {
}

http::ResponseContent::ResponseContent(const http::ResponseContent &other)
    : body_(other.body_), type_(other.type_) {
}

http::ResponseContent &http::ResponseContent::operator=(const http::ResponseContent &other) {
    if (this != &other) {
        body_ = other.body_;
        type_ = other.type_;
    }
    return *this;
}

const std::string &http::ResponseContent::getBody() const {
    return body_;
}

const std::string &http::ResponseContent::getType() const {
    return type_;
}

size_t http::ResponseContent::getContentLength() const {
    return body_.size();
}

http::ResponseContent::~ResponseContent() {
}

void http::ResponseContent::setFileType(const std::string &path, MimeTypes *mime) {
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        std::string extension = toLower(path.substr(pos + 1));
        type_ = mime->getMimeType(extension);
        return;
    }
    type_ = "text/plain"; // Default MIME type if no match is found
}
