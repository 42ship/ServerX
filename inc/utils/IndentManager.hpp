#pragma once

#include <iostream>

std::ostream &print_indent(std::ostream &os);

struct IndentManager {
    int modification;
};

std::ostream &operator<<(std::ostream &os, const IndentManager &mod);

const IndentManager indent = {1};    // Used as 'os << indent;'
const IndentManager unindent = {-1}; // Used as 'os << unindent;'
