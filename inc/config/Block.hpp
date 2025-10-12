#pragma once

#include <map>
#include <string>
#include <vector>

namespace config {

typedef std::vector<std::string> StringVector;
typedef std::map<std::string, StringVector> DirectiveMap;

/**
 * @class Block
 * @brief Base class for configuration blocks like 'server' and 'location'.
 *
 * This class provides a generic storage mechanism for configuration directives
 * using a map, allowing for high extensibility.
 */
class Block {
public:
    // ========================= Construction & Destruction =========================

    Block(std::string const &name);
    virtual ~Block();

    // ============================== Public Interface ==============================

    /** Checks if a directive exists. */
    bool has(std::string const &key) const;

    /** Safely gets directive values. Throws exception if not found. */
    StringVector const &get(std::string const &key) const;

    /** Adds a directive with a multiple value. */
    Block &add(std::string const &key, StringVector const &);
    Block &add(std::string const &key, std::string const &);
    Block &add(std::string const &key, std::string const &, std::string const &);

    // ============================== Getters & Setters =============================

    std::string const &name() const;
    std::string const &root() const;
    StringVector const &indexFiles() const;

    // ================================= Fluent API =================================

    Block &root(std::string const &);

    friend std::ostream &operator<<(std::ostream &, Block const &);

protected:
    friend class DirectiveHandler;

    std::string name_;
    DirectiveMap directives_;
};

} // namespace config
