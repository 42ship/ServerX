#pragma once

#include "config/arguments/IArgument.hpp"
#include "config/internal/types.hpp"

namespace config {

/**
 * @class Block
 * @brief Base class for configuration blocks like 'server' and 'location'.
 *
 * This class provides a generic storage mechanism for configuration directives
 * using a map, allowing for high extensibility.
 */
class Block {
public:
    Block(std::string const &name);
    Block(const Block &other);
    Block &operator=(const Block &other);
    virtual ~Block();

    // ============================== Public Interface ==============================

    /** Checks if a directive exists. */
    bool has(std::string const &key) const;

    /** Adds a directive with a multiple value. */
    Block &add(std::string const &key, std::vector<std::string> const &);
    Block &add(std::string const &key, std::string const &);
    Block &add(std::string const &key, std::string const &, std::string const &);

    // ============================== Getters & Setters =============================

    /** Safely gets directive values. Throws exception if not found. */
    ArgumentVector const *get(std::string const &key) const;
    std::vector<std::string> get(std::string const &key, http::HttpRequest const &req) const;
    std::string getFirstEvaluatedString(std::string const &key, http::HttpRequest const &req) const;

    std::string const &name() const;
    std::string root() const;
    std::vector<std::string> indexFiles() const;

    // ================================= Fluent API =================================

    Block &root(std::string const &);

protected:
    Block &add(std::string const &key, ParsedDirectiveArgs const &values);
    Block &add(std::string const &key, ArgumentVector const &values);
    std::vector<std::string> getRawValues(std::string const &key) const;
    std::string getFirstRawValue(std::string const &key) const;

    std::string name_;
    DirectiveMap directives_;

    friend class DirectiveHandler;
    friend std::ostream &operator<<(std::ostream &, Block const &);
    friend class ListenDirective;
};

} // namespace config
