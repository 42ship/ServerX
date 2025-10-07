#include "http/Handler.hpp"

#include <iterator>
#include <limits>
#include <unistd.h>

#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"
#include "http/utils.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"

namespace http {

namespace details {

std::string getUploadPath(const config::LocationBlock &l) {
    const config::StringVector *pv = l.get("upload_path");
    if (!pv || pv->empty()) {
        return "";
    }

    const std::string &path = (*pv)[0];
    if (path.empty()) {
        return "";
    }
    return path;
}

std::string buildUploadPath(const config::ServerBlock &s, const config::LocationBlock &l) {

    const std::string up = getUploadPath(l);
    if (up.empty()) {
        return "";
    }

    std::string root = s.getRoot();

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

// std::string getFilename(HttpRequest const &req, MimeTypes const &mime) {
//     std::string filename = req.getHeader("X-Filename");
//     std::string disposition = req.getHeader("Content-Disposition");
//     if (filename.empty()) {
//         if (disposition.empty()) {
//             throw exceptions::BadRequestException(
//                 "Missing filename (X-Filename or Content-Disposition)");
//         } else {
//             filename = extractFilename(disposition);
//         }
//     }
//     if (filename.empty()) {
//         throw exceptions::BadRequestException("Empty filename in Content-Disposition");
//     }
//     size_t index = filename.find_last_of(".");
//     if (index != std::string::npos && index == filename.size() - 1) {
//         throw exceptions::BadRequestException("Invalid filename (trailing dot)");
//     }

//     std::string contentType = req.getHeader("Content-Type");
//     if (contentType.empty()) {
//         throw exceptions::BadRequestException("Missing Content-Type");
//     }
//     if (contentType == "application/octet-stream") {
//         return filename;
//     }

//     std::string reqExt = mime.getMimeExt(contentType);
//     if (reqExt.empty() || contentType.find("multipart/form-data") != std::string::npos) {
//         throw exceptions::UnsupportedMediaTypeException();
//     }
//     std::string ext;
//     if (index != std::string::npos) {
//         ext = filename.substr(index + 1);
//     }
//     std::string mimeType = mime.getMimeType(ext);
//     if (ext == reqExt || contentType == mimeType) {
//         return filename;
//     }
//     if (index != std::string::npos) {
//         filename.resize(index + 1);
//     }
//     return filename + "." + reqExt;
// }

} // namespace details

FileUploadHandler::FileUploadHandler(MimeTypes const &mime) : mimeTypes_(mime) {}

HttpResponse FileUploadHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                       config::LocationBlock const *l) const {
    if (!l) {
        return error_pages::generateJsonErrorResponse(NOT_FOUND, req.version,
                                                      "No matching location for path: " + req.path);
    }

    std::string path = details::buildUploadPath(*s, *l);
    utils::ValidationResult vup = utils::validateUploadPath(path);
    if (!vup.result) {
        return error_pages::generateJsonErrorResponse(vup.status, req.version, vup.message);
    }

    utils::ValidationResult lim = utils::checkUploadLimit(req.getHeader("Content-Length"), *s);
    if (!lim.result) {
        return error_pages::generateJsonErrorResponse(lim.status, req.version, lim.message);
    }

    utils::ValidationResult pf = utils::parseFilename(req, mimeTypes_);
    if (!pf.result) {
        return error_pages::generateJsonErrorResponse(pf.status, req.version, pf.message);
    }

    path += pf.value;
    if (!utils::writeFile(req.body, path.c_str())) {
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version,
                                                      "Failed to write uploaded file to disk");
    }

    HttpResponse res(CREATED, req.version);
    std::string loc = l->getPath();
    if (!loc.empty() && loc[loc.size() - 1] != '/')
        loc += '/';
    // todo: refactor creating location header
    res.getHeaders()["Location"] = path.substr(path.find(loc));
    return res;
}

} // namespace http