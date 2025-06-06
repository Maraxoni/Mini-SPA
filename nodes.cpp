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

std::string get_node_type(Node *node) {
    if (dynamic_cast<Procedure *>(node)) {
        return "Procedure";
    } else if (dynamic_cast<WhileStmt *>(node)) {
        return "WhileStmt";
    } else if (dynamic_cast<IfStmt *>(node)) {
        return "IfStmt";
    } else if (dynamic_cast<Assign *>(node)) {
        return "Assign";
    } else if (dynamic_cast<Expr *>(node)) {
        return "Expr";
    } else if (dynamic_cast<Factor *>(node)) {
        return "Factor";
    } else if (dynamic_cast<Call *>(node)) {
        return "Call";
    }
    return "Unknown Node Type";
}
