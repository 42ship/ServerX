#include "utils/IndentManager.hpp"

static int getIndentIndex() {
    static int indent_index = std::ios_base::xalloc();
    return indent_index;
}

std::ostream &print_indent(std::ostream &os) {
    long indent_level = os.iword(getIndentIndex());
    for (long i = 0; i < indent_level; ++i) {
        os << "    "; // 4 spaces per indent level
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const IndentManager &mod) {
    os.iword(getIndentIndex()) += mod.modification;
    return os;
}
