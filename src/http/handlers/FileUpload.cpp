#include "common/filesystem.hpp"
#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"
#include "http/utils.hpp"
#include <unistd.h>

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
    if (up[0] == '/') {
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
