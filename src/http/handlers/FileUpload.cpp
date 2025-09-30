#include "http/Handler.hpp"

#include <iterator>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"
#include "utils/Logger.hpp"
#include "http/exceptions/HttpException.hpp"
#include "utils/utils.hpp"

namespace http {

namespace details {

std::string getUploadPath(const config::ServerBlock &s, const config::LocationBlock &l) {
    const config::StringVector *pv = l.get("upload_path");
    if (!pv || pv->empty()) {
        return "";
    }

    const std::string &up = (*pv)[0];
    if (up.empty()) {
        return "";
    }

    std::string root = s.getRoot();
    while (!root.empty() && root[root.size() - 1] == '/') {
        root.erase(root.size() - 1);
    }

    std::string path;
    if (!up.empty() && up[0] == '/') {
        path = up;
    } else if (!root.empty()) {
        path.reserve(root.size() + 1 + up.size() + 1);
        path = root;
        path += '/';
        path += up;
    } else {
        path = up;
    }

    while (!path.empty() && path[path.size() - 1] == '/') {
        path.erase(path.size() - 1);
    }
    path += '/';
    return path;
}

std::string trim(const std::string &s) {
    std::string::size_type start = s.find_first_not_of(" \t");
    std::string::size_type end = s.find_last_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, end - start + 1);
}

std::string extractFilename(const std::string &disposition) {
    std::istringstream ss(disposition);
    std::string token;

    while (std::getline(ss, token, ';')) {
        token = trim(token);
        std::string::size_type pos = token.find("filename=");
        if (pos != std::string::npos) {
            std::string filename = token.substr(pos + 9); // 9 = strlen("filename=")
            if (!filename.empty() && filename[0] == '"') {
                filename.erase(0, 1);
            }
            if (!filename.empty() && filename[filename.size() - 1] == '"') {
                filename.erase(filename.size() - 1);
            }
            return filename;
        }
    }
    return "";
}

std::string getFilename(HttpRequest const &req, MimeTypes const &mime) {
    std::string filename = req.getHeader("X-Filename");
    std::string disposition = req.getHeader("Content-Disposition");
    if (filename.empty()) {
        if (disposition.empty()) {
            throw exceptions::BadRequestException();
        } else {
            filename = extractFilename(disposition);
        }
    }
    std::string contentType = req.getHeader("Content-Type");
    if (contentType == "application/octet-stream") {
        return filename;
    }
    std::string ext = filename.substr(filename.find_last_of(".") + 1);
    if (contentType.empty()) {
        throw exceptions::BadRequestException();
    }
    std::string reqExt = mime.getMimeExt(contentType);
    if (reqExt.empty()) {
        throw exceptions::UnsupportedMediaTypeException();
    }
    std::string mimeType = mime.getMimeType(ext);
    LOG_DEBUG("Content-Type: " + contentType + "MimeType: " + mimeType);
    if (ext == reqExt || contentType == mimeType) {
        return filename;
    }
    filename = filename.substr(0, filename.find_last_of(".") + 1);
    return filename + reqExt;
}

inline bool isDir(const std::string &p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

} // namespace details

static inline bool checkLimit(size_t contentLength, config::ServerBlock const &s)
{
    const config::StringVector *sv = s.get("upload_file_size");
    if (!sv || sv->empty()) {
        return true;
    }
    std::string sUploadFileSize = (*sv)[0];
    size_t uploadFileSize = utils::fromString<size_t>(sUploadFileSize);
    uploadFileSize = uploadFileSize * 1024 * 1024;
    if (contentLength > uploadFileSize) {
        return false;
    }
    return true;
}

FileUploadHandler::FileUploadHandler(MimeTypes const &mime) : mimeTypes_(mime) {
}

HttpResponse FileUploadHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                       config::LocationBlock const *l) const {
    if (!l) {
        return error_pages::generateJsonErrorResponse(NOT_FOUND, req.version);
    }
    std::string path = details::getUploadPath(*s, *l);
    if (path.empty()) {
        return error_pages::generateJsonErrorResponse(METHOD_NOT_ALLOWED, req.version);
    }
    if (!details::isDir(path)) {
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version);
    }
    if (access(path.c_str(), W_OK | X_OK) == -1) {
        if (errno == EACCES || errno == EROFS) {
            return error_pages::generateJsonErrorResponse(FORBIDDEN, req.version);
        }
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version);
    }
    if (!checkLimit(req.body.length(), *s)) {
        return error_pages::generateJsonErrorResponse(PAYLOAD_TOO_LARGE, req.version);
    }
    std::string filename;
    try {
        filename = details::getFilename(req, mimeTypes_);
    } catch (const exceptions::BadRequestException &e) {
        LOG_ERROR(e.what());
        return error_pages::generateJsonErrorResponse(BAD_REQUEST, req.version);
    } catch (const exceptions::UnsupportedMediaTypeException &e) {
        LOG_ERROR(e.what());
        return error_pages::generateJsonErrorResponse(UNSUPPORTED_MEDIA_TYPE, req.version);
    } catch (const exceptions::HttpException &e) {
        LOG_ERROR(e.what());
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version);
    }
    LOG_TRACE("filename: " + filename);
    path += filename;

    LOG_TRACE("PATH: " + path);
    if (!utils::writeFile(req.body, path.c_str())) {
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version);
    }
    HttpResponse res(CREATED, req.version);
    res.getHeaders()["Location"] = l->getPath() + filename;
    return res;
}

} // namespace http