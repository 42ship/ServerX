#include "http/handlers/DirectoryListingHandler.hpp"
#include "common/filesystem.hpp"
#include "config/LocationBlock.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include <sstream>
#include <unistd.h>

namespace http {

namespace {

std::string formatSize(off_t size) {
    std::ostringstream ss;
    if (size < 1024)
        ss << size << " B";
    else
        ss << (size / 1024) << " KB";
    return ss.str();
}

std::string formatTime(time_t mtime) {
    char timeBuf[64];
    struct tm *tm = localtime(&mtime);
    strftime(timeBuf, sizeof(timeBuf), "%d-%b-%Y %H:%M", tm);
    return std::string(timeBuf);
}

std::string generateListingHtml(std::string const &path,
                                const std::vector<utils::FileEntry> &files) {
    std::ostringstream html;

    html << "<html>\n<head><title>Index of " << path << "</title></head>\n"
         << "<body>\n<h1>Index of " << path << "</h1><hr><pre>\n";

    // Add parent directory link if not at root
    if (path != "/") {
        html << "<a href=\"../\">../</a>\n";
    }

    for (size_t i = 0; i < files.size(); ++i) {
        const utils::FileEntry &file = files[i];

        std::string name = file.name;
        if (file.isDir)
            name += "/";

        std::string displayName = name;
        if (displayName.size() > 50) {
            displayName = displayName.substr(0, 47) + "..>";
        }

        html << "<a href=\"" << name << "\">" << displayName << "</a>";

        int padding = 51 - displayName.size();
        if (padding > 0)
            html << std::string(padding, ' ');

        html << formatTime(file.mtime) << std::string(20, ' ');
        html << (file.isDir ? "-" : formatSize(file.size)) << "\n";
    }

    html << "</pre><hr></body>\n</html>";
    return html.str();
}

} // namespace

void DirectoryListingHandler::handle(Request const &req, Response &res, MimeTypes const &mime) {
    std::string fullPath = req.resolvePath();
    struct stat st;

    if (!utils::getFileStatus(fullPath, st)) {
        return StaticFileHandler::handle(req, res, mime);
    }

    if (!S_ISDIR(st.st_mode)) {
        return StaticFileHandler::handle(req, res, mime);
    }

    std::string indexPath = req.location()->resolveIndexFile(fullPath);
    if (!indexPath.empty()) {
        return StaticFileHandler::handle(req, res, mime);
    }

    if (req.path().empty() || req.path()[req.path().size() - 1] != '/') {
        return StaticFileHandler::handle(req, res, mime);
    }

    if (access(fullPath.c_str(), R_OK | X_OK) != 0) {
        res.status(FORBIDDEN);
        return;
    }

    std::vector<utils::FileEntry> files;
    if (!utils::getDirectoryEntries(fullPath, files)) {
        res.status(INTERNAL_SERVER_ERROR);
        return;
    }

    std::string html = generateListingHtml(req.path(), files);

    res.status(OK);
    res.setBodyInMemory(html, "text/html");
}

} // namespace http
