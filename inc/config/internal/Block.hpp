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
    virtual ~Block() {
    }

    /**
     * @brief Provides read-only access to the underlying directive map.
     */
    DirectiveMap const &getDirectives() const {
        return directives_;
    }

    /**
     * @brief Provides read-write access to the underlying directive map.
     * (Primarily for use by the ConfigBuilder).
     */
    DirectiveMap &getDirectives() {
        return directives_;
    }

    /**
     * @brief Retrieves the arguments for a specific directive.
     * @param key The name of the directive (e.g., "root").
     * @return A const pointer to the vector of arguments, or NULL if the
     * directive is not found or has no arguments.
     */
    StringVector const *get(std::string const &key) const {
        DirectiveMap::const_iterator it = directives_.find(key);
        if (it != directives_.end() && !it->second.empty())
            return &it->second;
        return NULL;
    }

    /** @brief Checks if a directive exists within the block. */
    bool has(std::string const &key) const {
        return directives_.find(key) != directives_.end();
    }

    // --- Common Directive Accessors ---

    /**
     * @brief A convenient, strongly-typed accessor for the 'root' directive.
     * @return The root path if set, otherwise an empty string.
     */
    std::string getRoot() const {
        StringVector const *args = get("root");
        if (args)
            return (*args)[0];
        return "";
    }

protected:
    DirectiveMap directives_;
};

} // namespace config
