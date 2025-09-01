#pragma once

#include <string>
#include <vector>

namespace config {

/**
 * @class LocationBlock
 * @brief Represents a single 'location' block from the configuration file.
 *
 * This class is a strongly-typed container for the directives defined within a
 * location context. It holds settings such as the document root and index files
 * for a specific URI path.
 */
class LocationBlock {
public:
    std::string path; //!< The URI path this location block matches (e.g., "/images/").
    std::string root; //!< The root directory for requests matching this location.
    std::vector<std::string> index;    //!< The list of index files to search for.
    std::vector<std::string> cgi_pass; //!< Path to CGI interpreter and script.

    bool hasCgiPass() const {
        return !cgi_pass.empty();
    }
};

} // namespace config
