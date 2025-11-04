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

    /**
     * @brief Checks if a directive exists within the block.
     * @param key The name of the directive to check.
     * @return True if the directive exists, false otherwise.
     */
    bool has(std::string const &key) const;

    /**
     * @brief Safely gets the values for a directive.
     * @param key The name of the directive.
     * @return A constant reference to the vector of directive values.
     * @throws std::out_of_range If the directive is not found.
     */
    ArgumentVector const &get(std::string const &key) const;

    /**
     * @brief Adds a directive with multiple values.
     * @param key The name of the directive.
     * @param values A vector of string values.
     * @return A reference to the Block object for chaining.
     */
    Block &add(std::string const &key, std::vector<std::string> const &);
    /** @copydoc add(std::string const &, StringVector const &) */
    Block &add(std::string const &key, std::string const &);
    /** @copydoc add(std::string const &, StringVector const &) */
    Block &add(std::string const &key, std::string const &, std::string const &);

    std::vector<std::string> getRawValues(std::string const &key) const;

    std::string getFirstRawValue(std::string const &key) const;

    // ============================== Getters & Setters =============================

    std::vector<std::string> get(std::string const &key, http::Request const &req) const;
    std::string getFirstEvaluatedString(std::string const &key, http::Request const &req) const;

    /**
     * @brief Gets the name of the block.
     * @return A constant reference to the block's name.
     */
    std::string const &name() const;

    /**
     * @brief Gets the root path directive.
     * @return The root path string, or an empty string if not set.
     */
    std::string root() const;

    /**
     * @brief Gets the index files directive.
     * @return A vector of index file names.
     */
    std::vector<std::string> indexFiles() const;

    // ================================= Fluent API =================================

    /**
     * @brief Sets the root directive using a fluent interface.
     * @param path The root path to set.
     * @return A reference to the Block object for chaining.
     */
    Block &root(std::string const &);

protected:
    Block &add(std::string const &key, ParsedDirectiveArgs const &values);
    Block &add(std::string const &key, ArgumentVector const &values);

    std::string name_;        //!< The name of the block (e.g., "server", "location").
    DirectiveMap directives_; //!< Map storing directive names and their values.
    friend class DirectiveHandler;
    friend std::ostream &operator<<(std::ostream &, Block const &);
    friend class ListenDirective;
};

} // namespace config
