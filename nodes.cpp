//
// Created by Kaspek on 02.04.2025.
//

#include "nodes.h"

bool simple_semantic_utils::verify_name(const std::string &name) {
    if (name.empty()) {
        return false;
    }

    if (!isalpha(name[0])) {
        return false;
    }

    for (char c: name) {
        if (!isalnum(c)) {
            return false;
        }
    }

    return true;
}

bool simple_semantic_utils::verify_integer(const std::string &value) {
    if (value.empty()) {
        return false;
    }

    for (char c: value) {
        if (!isdigit(c)) {
            return false;
        }
    }

    return true;
}
