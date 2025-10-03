#include "config/internal/Block.hpp"
#include "config/arguments/String.hpp"
#include "utils/IndentManager.hpp"

namespace config {

Block::Block(std::string const &name) : name_(name) {}

Block::~Block() {
    for (DirectiveMap::iterator it = directives_.begin(); it != directives_.end(); ++it) {
        for (ArgumentVector::iterator a_it = it->second.begin(); a_it != it->second.end(); ++a_it) {
            delete *a_it;
        }
    }
}

Block::Block(const Block &other) : name_(other.name_) {
    for (DirectiveMap::const_iterator it = other.directives_.begin(); it != other.directives_.end();
         ++it) {
        ArgumentVector new_args;
        for (size_t i = 0; i < it->second.size(); ++i) {
            new_args.push_back(it->second[i]->clone());
        }
        directives_[it->first] = new_args;
    }
}

Block &Block::operator=(const Block &other) {
    if (this == &other) {
        return *this;
    }

    for (DirectiveMap::iterator it = directives_.begin(); it != directives_.end(); ++it) {
        for (ArgumentVector::iterator a_it = it->second.begin(); a_it != it->second.end(); ++a_it) {
            delete *a_it;
        }
    }
    directives_.clear();

    name_ = other.name_;
    for (DirectiveMap::const_iterator it = other.directives_.begin(); it != other.directives_.end();
         ++it) {
        ArgumentVector new_args;
        for (size_t i = 0; i < it->second.size(); ++i) {
            new_args.push_back(it->second[i]->clone());
        }
        directives_[it->first] = new_args;
    }
    return *this;
}

/**
 * @brief Provides read-only access to the underlying directive map.
 */
DirectiveMap const &Block::getDirectives() const { return directives_; }

/**
 * @brief Provides read-write access to the underlying directive map.
 * (Primarily for use by the ConfigBuilder).
 */
DirectiveMap &Block::getDirectives() { return directives_; }

ArgumentVector const *Block::operator[](std::string const &key) const { return get(key); }

ArgumentVector &Block::operator[](std::string const &key) { return directives_[key]; }

/**
 * @brief Retrieves the arguments for a specific directive.
 * @param key The name of the directive (e.g., "root").
 * @return A const pointer to the vector of arguments, or NULL if the
 * directive is not found or has no arguments.
 */
ArgumentVector const *Block::get(std::string const &key) const {
    DirectiveMap::const_iterator it = directives_.find(key);
    if (it != directives_.end() && !it->second.empty())
        return &it->second;
    return NULL;
}

/** @brief Checks if a directive exists within the block. */
bool Block::has(std::string const &key) const { return directives_.find(key) != directives_.end(); }

void Block::add(std::string const &key, ArgumentVector const &values) { directives_[key] = values; }

void Block::add(std::string const &key, std::string const &value) {
    ArgumentVector v;
    v.push_back(new String(value));
    add(key, v);
}

// --- Common Directive Accessors ---

/**
 * @brief A convenient, strongly-typed accessor for the 'root' directive.
 * @return The root path if set, otherwise an empty string.
 */
std::string Block::getRoot() const {
    ArgumentVector const *args = get("root");
    if (args)
        return (*args)[0]->getRawValue();
    return "";
}

std::string const &Block::getName() const { return name_; }

void Block::setRoot(std::string const &root) {
    DirectiveMap::iterator it = directives_.find("root");
    if (it != directives_.end()) {
        String *strArg = dynamic_cast<String *>(it->second[0]);
        if (strArg) {
            strArg->setValue(root);
        } else {
            delete it->second[0];
            it->second[0] = new String(root);
        }
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
