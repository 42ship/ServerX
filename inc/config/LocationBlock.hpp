#pragma once

#include "internal/Block.hpp"
#include <ostream>

namespace config {

/**
 * @class LocationBlock
 * @brief Represents a single 'location' block from the configuration file.
 *
 * This class is a strongly-typed container for the directives defined within a
 * location context. It holds settings such as the document root and index files
 * for a specific URI path.
 */
class LocationBlock : public Block {
public:
    std::string const &getPath() const;
    void setPath(std::string const &);
    StringVector const *getIndexFiles() const;
    bool hasCgiPass() const;

private:
    friend class Mapper;
    std::string path_;
};

std::ostream &operator<<(std::ostream &o, LocationBlock const &t);

} // namespace config
