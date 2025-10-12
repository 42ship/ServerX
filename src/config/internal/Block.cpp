#include "config/Block.hpp"
#include "utils/IndentManager.hpp"
#include <stdexcept>

namespace config {

Block::Block(std::string const &name) : name_(name) {}

Block::~Block() {}

/**
 * @brief Retrieves the arguments for a specific directive.
 * @param key The name of the directive (e.g., "root").
 * @return A const pointer to the vector of arguments, or NULL if the
 * directive is not found or has no arguments.
 */
StringVector const &Block::get(std::string const &key) const {
    DirectiveMap::const_iterator it = directives_.find(key);
    if (it != directives_.end() && !it->second.empty())
        return it->second;
    throw std::out_of_range(key);
}

/** @brief Checks if a directive exists within the block. */
bool Block::has(std::string const &key) const { return directives_.find(key) != directives_.end(); }

Block &Block::add(std::string const &key, StringVector const &values) {
    directives_[key] = values;
    return *this;
}

Block &Block::add(std::string const &key, std::string const &value) {
    StringVector v;
    v.push_back(value);
    add(key, v);
    return *this;
}

Block &Block::add(std::string const &key, std::string const &v1, std::string const &v2) {
    StringVector v;
    v.push_back(v1);
    v.push_back(v2);
    add(key, v);
    return *this;
}

std::string const &Block::root() const {
    if (!has("root")) {
        static const std::string empty;
        return empty;
    }
    return get("root")[0];
}

Block &Block::root(std::string const &root) {
    DirectiveMap::iterator it = directives_.find("root");
    if (it != directives_.end()) {
        it->second[0] = root;
    } else
        add("root", root);
    return *this;
}

StringVector const &Block::indexFiles() const {
    if (!has("index")) {
        static const StringVector empty;
        return empty;
    }
    return get("index");
}

std::string const &Block::name() const { return name_; }

std::ostream &operator<<(std::ostream &o, Block const &b) {
    DirectiveMap const &m = b.directives_;
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
