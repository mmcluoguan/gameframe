#include "3rd/cppitertools/filterfalse.hpp"

#include <iostream>
#include <string>
#include <vector>

bool greater_than_four(int i)
{
    return i > 4;
}

int main4()
{
    std::vector<int> ns { 1, 5, 6, 0, 7, 2, 3, 8, 3, 0, 2, 1 };
    std::cout << "ns = { ";
    for (auto&& i : ns) {
        std::cout << i << ' ';
    }
    std::cout << "}\n";

    // like filter() but only shows elements that are false under the predicate
    std::cout << "Greater than 4\n";
    for (auto&& i : iter::filterfalse(greater_than_four, ns)) {
        std::cout << i << '\n';
    }

    // single argument version only shows falsey values
    std::cout << "filterfalse(ns):\n";
    for (auto&& i : iter::filterfalse(ns)) {
        std::cout << i << '\n';
    }

    // only print non-empty strings
    std::vector<std::string> words { "hello", "", "", "world", "", "goodbye", "" };
    for (auto&& s : iter::filterfalse(&std::string::empty, words)) {
        std::cout << s << '\n';
    }
}
