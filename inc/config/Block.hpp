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
    StringVector const &get(std::string const &key) const;

    /**
     * @brief Adds a directive with multiple values.
     * @param key The name of the directive.
     * @param values A vector of string values.
     * @return A reference to the Block object for chaining.
     */
    Block &add(std::string const &key, StringVector const &);
    /** @copydoc add(std::string const &, StringVector const &) */
    Block &add(std::string const &key, std::string const &);
    /** @copydoc add(std::string const &, StringVector const &) */
    Block &add(std::string const &key, std::string const &, std::string const &);

    // ============================== Getters & Setters =============================

    /**
     * @brief Gets the name of the block.
     * @return A constant reference to the block's name.
     */
    std::string const &name() const;

    /**
     * @brief Gets the root path directive.
     * @return The root path string, or an empty string if not set.
     */
    std::string const &root() const;

    /**
     * @brief Gets the index files directive.
     * @return A vector of index file names.
     */
    StringVector const &indexFiles() const;

    // ================================= Fluent API =================================

    /**
     * @brief Sets the root directive using a fluent interface.
     * @param path The root path to set.
     * @return A reference to the Block object for chaining.
     */
    Block &root(std::string const &);

    friend std::ostream &operator<<(std::ostream &, Block const &);

protected:
    friend class DirectiveHandler;

    std::string name_;        //!< The name of the block (e.g., "server", "location").
    DirectiveMap directives_; //!< Map storing directive names and their values.
};

} // namespace config
