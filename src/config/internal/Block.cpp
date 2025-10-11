#include "config/internal/Block.hpp"
#include "utils/IndentManager.hpp"

namespace config {

Block::Block(std::string const &name) : name_(name) {}

Block::~Block() {}

/**
 * @brief Provides read-only access to the underlying directive map.
 */
DirectiveMap const &Block::getDirectives() const { return directives_; }

/**
 * @brief Provides read-write access to the underlying directive map.
 * (Primarily for use by the ConfigBuilder).
 */
DirectiveMap &Block::getDirectives() { return directives_; }

/**
 * @brief Retrieves the arguments for a specific directive.
 * @param key The name of the directive (e.g., "root").
 * @return A const pointer to the vector of arguments, or NULL if the
 * directive is not found or has no arguments.
 */
StringVector const *Block::get(std::string const &key) const {
    DirectiveMap::const_iterator it = directives_.find(key);
    if (it != directives_.end() && !it->second.empty())
        return &it->second;
    return NULL;
}

/** @brief Checks if a directive exists within the block. */
bool Block::has(std::string const &key) const { return directives_.find(key) != directives_.end(); }

void Block::add(std::string const &key, StringVector const &values) { directives_[key] = values; }

void Block::add(std::string const &key, std::string const &value) {
    StringVector v;
    v.push_back(value);
    add(key, v);
}

// --- Common Directive Accessors ---

/**
 * @brief A convenient, strongly-typed accessor for the 'root' directive.
 * @return The root path if set, otherwise an empty string.
 */
std::string Block::getRoot() const {
    StringVector const *args = get("root");
    if (args)
        return (*args)[0];
    return "";
}

std::string const &Block::getName() const { return name_; }

void Block::setRoot(std::string const &root) {
    DirectiveMap::iterator it = directives_.find("root");
    if (it != directives_.end()) {
        it->second[0] = root;
    } else
        add("root", root);
}

std::ostream &operator<<(std::ostream &o, Block const &b) {
    DirectiveMap const &m = b.getDirectives();
    for (DirectiveMap::const_iterator it = m.begin(); it != m.end(); ++it) {
        o << print_indent << it->first << ":";
        for (size_t i = 0; i < it->second.size(); i++) {
            o << " '" << it->second[i] << "'";
        }
        o << "\n";
    }
    return o;
}

} // namespace config
