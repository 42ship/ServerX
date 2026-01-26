#include "config/Block.hpp"
#include "config/arguments/ArgumentFactory.hpp"
#include "config/arguments/Integer.hpp"
#include "config/arguments/String.hpp"
#include "config/internal/types.hpp"
#include "utils/IndentManager.hpp"
#include <stdexcept>

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

std::vector<std::string> Block::get(std::string const &key, http::Request const &req) const {
    ArgumentVector const &argv = get(key);
    std::vector<std::string> res;
    res.reserve(argv.size());
    for (ArgumentVector::const_iterator it = argv.begin(); it != argv.end(); ++it) {
        if (*it)
            res.push_back((*it)->evaluate(req));
    }
    return res;
}

std::string Block::getFirstEvaluatedString(std::string const &key, http::Request const &req) const {
    ArgumentVector const &argv = get(key);
    if (argv.empty()) {
        return "";
    }
    return argv.front()->evaluate(req);
}

/**
 * @brief Retrieves the arguments for a specific directive.
 * @param key The name of the directive (e.g., "root").
 * @return A const pointer to the vector of arguments, or NULL if the
 * directive is not found or has no arguments.
 */
ArgumentVector const &Block::get(std::string const &key) const {
    DirectiveMap::const_iterator it = directives_.find(key);
    if (it != directives_.end())
        return it->second;
    throw std::out_of_range(key);
}

/** @brief Checks if a directive exists within the block. */
bool Block::has(std::string const &key) const { return directives_.count(key); }

Block &Block::add(std::string const &key, ArgumentPtr value) {
    if (value)
        directives_[key].push_back(value);
    return *this;
}

Block &Block::add(std::string const &key, ArgumentVector const &values) {
    DirectiveMap::const_iterator it = directives_.find(key);
    if (it != directives_.end()) {
        for (size_t i = 0; i < it->second.size(); i++) {
            delete it->second[i];
        }
    }
    directives_[key] = values;
    return *this;
}

Block &Block::add(std::string const &key, ParsedDirectiveArgs const &args) {
    add(key, ArgumentFactory::get(args));
    return *this;
}

Block &Block::add(std::string const &key, std::vector<std::string> const &value) {
    if (value.empty())
        return *this;
    ArgumentVector v;
    v.reserve(value.size());
    for (size_t i = 0; i < value.size(); i++) {
        v.push_back(new String(value[i]));
    }
    add(key, v);
    return *this;
}

Block &Block::add(std::string const &key, std::string const &value) {
    ArgumentVector v;
    v.push_back(new String(value));
    add(key, v);
    return *this;
}

Block &Block::add(std::string const &key, std::string const &v1, std::string const &v2) {
    ArgumentVector v;
    v.push_back(new String(v1));
    v.push_back(new String(v2));
    add(key, v);
    return *this;
}

std::string Block::root() const { return has("root") ? getFirstRawValue("root") : ""; }

std::string const &Block::name() const { return name_; }

std::vector<std::string> Block::indexFiles() const { return getRawValues("index"); }

Block &Block::root(std::string const &root) {
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
    return *this;
}

std::vector<std::string> Block::getRawValues(std::string const &key) const {
    std::vector<std::string> res;
    ArgumentVector const &matched = get(key);
    res.reserve(matched.size());
    for (size_t i = 0; i < matched.size(); i++) {
        res.push_back(matched[i]->getRawValue());
    }
    return res;
}

std::string Block::getFirstRawValue(std::string const &key) const {
    ArgumentVector const &matched = get(key);
    return matched[0]->getRawValue();
}

std::ostream &operator<<(std::ostream &o, Block const &b) {
    DirectiveMap const &m = b.directives_;
    for (DirectiveMap::const_iterator it = m.begin(); it != m.end(); ++it) {
        o << printIndent << it->first << ":";
        for (size_t i = 0; i < it->second.size(); i++) {
            o << " '" << it->second[i]->getRawValue() << "'";
        }
        o << "\n";
    }
    return o;
}

size_t Block::maxBodySize() const {
    std::string const key = "client_max_body_size";
    if (!has(key))
        return 0;
    ArgumentVector const &argv = get(key);
    if (argv.empty())
        return 0;
    Integer const *arg = dynamic_cast<Integer const *>(argv[0]);
    if (!arg)
        return 0;
    return arg->getIntValue();
}

} // namespace config
