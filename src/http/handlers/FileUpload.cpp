#include "common/filesystem.hpp"
#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"
#include "http/utils.hpp"
#include <unistd.h>
#include <sstream>

namespace http {

namespace details {

std::string getUploadPath(const config::LocationBlock &l) {
    if (!l.has("upload_path")) {
        return "";
    }

    const config::StringVector &v = l.get("upload_path");
    if (v.empty()) {
        return "";
    }

    const std::string &path = v[0];
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

    std::string root = s.root();

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

HttpResponse FileUploadHandler::handleMultipartFormData(HttpRequest const &req, config::ServerBlock const *s,
                                            config::LocationBlock const *l) const {
    std::string boundary = utils::extractHeaderParam(req.getHeader("Content-Type"), "boundary=");
    if (boundary.empty()) {
        return error_pages::generateJsonErrorResponse(BAD_REQUEST, req.version, "Missing or invalid boundary parameter in Content-Type header");
    }

    boundary = "--" + boundary;
    std::istringstream stream(req.body);

    HttpResponse firstRes;
    for (HttpRequest multiReq = details::parse(stream, boundary); stream.good() ; multiReq = details::parse(stream, boundary))
    {
        multiReq.uri = req.uri;
        multiReq.version = req.version;
        multiReq.headers["Content-Length"] = req.getHeader("Content-Length");
        multiReq.path = req.path;
        HttpResponse res = handle(multiReq, s, l);
        if (firstRes.getBodyType() == BODY_NONE) {
            firstRes = res;
        }
        if (res.getStatus() != CREATED) {
            return res;
        }
    }
    return firstRes;
}

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

    std::string transferEncoding = req.getHeader("Transfer-Encoding");
    if (!transferEncoding.empty() && transferEncoding.find("chunked") != std::string::npos) {
        return error_pages::generateJsonErrorResponse(
            http::NOT_IMPLEMENTED, req.version, "Transfer-Encoding: chunked is not supported");
    }

    std::string contentLen = req.getHeader("Content-Length");
    utils::ValidationResult len = utils::checkContentLength(contentLen);
    if (!len.result) {
        return error_pages::generateJsonErrorResponse(len.status, req.version, len.message);
    }

    utils::ValidationResult lim = utils::checkUploadLimit(contentLen, *s);
    if (!lim.result) {
        return error_pages::generateJsonErrorResponse(lim.status, req.version, lim.message);
    }

    std::string contentType = req.getHeader("Content-Type");
    if (contentType.find("multipart/form-data") != std::string::npos) {
        return handleMultipartFormData(req, s, l);
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
    std::string loc = l->path();
    if (!loc.empty() && loc[loc.size() - 1] != '/')
        loc += '/';
    // todo: refactor creating location header
    res.getHeaders()["Location"] = path.substr(path.find(loc));
    return res;
}

} // namespace http
