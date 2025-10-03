#pragma once

#include "internal/Block.hpp"
#include <ostream>

namespace config {

class ServerBlock;
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
    LocationBlock();
    std::string const &getPath() const;
    void setPath(std::string const &);
    ArgumentVector const *getIndexFiles() const;
    bool hasCgiPass() const;

    void setParent(ServerBlock *parent);
    ServerBlock *getParent();

private:
    ServerBlock *parent_;
    std::string path_;
};

std::ostream &operator<<(std::ostream &o, LocationBlock const &t);

} // namespace config
