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

    virtual ~Block();

    // ============================== Public Interface ==============================

    /** @brief Checks if a directive exists within the block. */
    bool has(std::string const &key) const;
    StringVector const *operator[](std::string const &key) const;
    void add(std::string const &key, StringVector const &values);
    void add(std::string const &key, std::string const &value);

    // ============================== Getters & Setters =============================

    /** @brief Provides read-only access to the underlying directive map. */
    DirectiveMap const &getDirectives() const;

    /** @brief Provides read-write access to the underlying directive map. */
    DirectiveMap &getDirectives();

    /**
     * @brief Retrieves the arguments for a specific directive.
     * @param key The name of the directive (e.g., "root").
     * @return A const pointer to the vector of arguments, or NULL if the
     * directive is not found or has no arguments.
     */
    StringVector const *get(std::string const &key) const;

    /**
     * @brief A convenient, strongly-typed accessor for the 'root' directive.
     * @return The root path if set, otherwise an empty string.
     */
    std::string getRoot() const;

    void setRoot(std::string const &root);

protected:
    friend class DirectiveHandler;

    DirectiveMap directives_;
};

} // namespace config
