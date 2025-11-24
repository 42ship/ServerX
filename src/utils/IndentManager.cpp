#include "utils/IndentManager.hpp"

static int getIndentIndex() {
    static int indentIndex = std::ios_base::xalloc();
    return indentIndex;
}

std::ostream &printIndent(std::ostream &os) {
    long indentLevel = os.iword(getIndentIndex());
    for (long i = 0; i < indentLevel; ++i) {
        os << "    "; // 4 spaces per indent level
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const IndentManager &mod) {
    os.iword(getIndentIndex()) += mod.modification;
    return os;
}
