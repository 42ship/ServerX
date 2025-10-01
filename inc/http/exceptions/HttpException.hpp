#pragma once

#include <string>
#include <exception>

namespace http {
namespace exceptions {
class HttpException : public std::exception {
protected:
    std::string message_;

public:
    HttpException() : message_("HTTP error") {
    }
    HttpException(std::string const &message) : message_(message) {
    }
    virtual const char *what() const throw() {
        return message_.c_str();
    }
    virtual ~HttpException() throw() {
    }
};

class InternalServerErrorException : public HttpException {
public:
    InternalServerErrorException() : HttpException("Internal server error") {
    }
    InternalServerErrorException(std::string const &message) : HttpException(message) {
    }
};

class BadRequestException : public HttpException {
public:
    BadRequestException() : HttpException("Bad request error") {
    }
    BadRequestException(std::string const &message) : HttpException(message) {
    }
};

class UnsupportedMediaTypeException : public HttpException {
public:
    UnsupportedMediaTypeException() : HttpException("Unsupported media type error") {
    }
    UnsupportedMediaTypeException(std::string const &message) : HttpException(message) {
    }
};
} // namespace exceptions
} // namespace http