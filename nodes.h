#ifndef MINISPA_NODES_H
#define MINISPA_NODES_H

#include <string>
#include <vector>
#include <memory>

#include "utils.h"

namespace simple_semantic_utils {
    //NAME : LETTER (LETTER | DIGIT)*
    bool verify_name(const std::string &name);

    //INTEGER: DIGIT+
    bool verify_integer(const std::string &value);

}
class Node {
public:
    virtual ~Node() = default;

    [[nodiscard]] virtual std::string to_string() const = 0;

    //TODO: probably remove verify() instead use verify just before adding to the tree
    virtual bool verify() const {
        return true;
    }
};

// stmtLst : stmt+
// stmt : assign | while

// procedure : ‘procedure’ proc_name ‘{‘ stmtLst ‘}’
class Procedure : public Node {
public:
    std::string name;
    std::vector<std::shared_ptr<Node>> stmt_list;

    Procedure(const std::string &name, const std::vector<std::shared_ptr<Node>> &stmt_list) {
        this->name = name;
        this->stmt_list = stmt_list;
    }

    [[nodiscard]] std::string to_string() const override {
        std::string result = "===== PROCEDURE  " + name + " {\n";
        for (const auto &stmt: stmt_list) {
            result += stmt->to_string() + "\n";
        }
        result += "}\n===== END PROCEDURE " + name;
        return result;
    }

    [[nodiscard]] bool verify() const override {
        if (!simple_semantic_utils::verify_name(name)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid procedure name: " + name);
            return false;
        }

        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Procedure " + name + " has no statements");
            return false;
        }

        return true;
    }
};

// while : ‘while’ var_name ‘{‘ stmtLst ‘}’
class WhileStmt : public Node {
public:
    std::string var_name;
    std::vector<std::shared_ptr<Node>> stmt_list;

    WhileStmt(const std::string &var_name, const std::vector<std::shared_ptr<Node>> &stmt_list) {
        this->var_name = var_name;
        this->stmt_list = stmt_list;
    }

    [[nodiscard]] std::string to_string() const override {
        std::string result = "=== WHILE " + var_name + " {\n";
        for (const auto &stmt: stmt_list) {
            result += stmt->to_string() + "\n";
        }
        result += "}\n=== END WHILE " + var_name;
        return result;
    }

    [[nodiscard]] bool verify() const override {
        if (!simple_semantic_utils::verify_name(var_name)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid variable name: " + var_name);
            return false;
        }

        if (stmt_list.empty()) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "While statement " + var_name + " has no statements");
            return false;
        }

        return true;
    }
};

// assign : var_name ‘=’ expr ‘;’
class Assign : public Node {
public:
    std::string var_name;
    std::shared_ptr<Node> expr;

    Assign(const std::string &var_name, const std::shared_ptr<Node> &expr) {
        this->var_name = var_name;
        this->expr = expr;
    }

    [[nodiscard]] std::string to_string() const override {
        return var_name + " = " + expr->to_string() + ";";
    }

    [[nodiscard]] bool verify() const override {
        if (!simple_semantic_utils::verify_name(var_name)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Invalid variable name: " + var_name);
            return false;
        }

        if (expr == nullptr) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Assign statement " + var_name + " has no expression");
            return false;
        }

        return true;
    }
};

// expr : expr ‘+’ factor | factor
class Expr : public Node {
public:
    std::shared_ptr<Node> left;
    char op;
    std::shared_ptr<Node> right;

    Expr(const std::shared_ptr<Node> &left, char op, const std::shared_ptr<Node> &right) {
        this->left = left;
        this->op = op;
        this->right = right;
    }

    [[nodiscard]] std::string to_string() const override {
        return left->to_string() + " " + op + " " + right->to_string();
    }

    [[nodiscard]] bool verify() const override {
        if (left == nullptr) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expr has no left node");
            return false;
        }

        if (right == nullptr) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expr has no right node");
            return false;
        }

        return true;
    }
};

// NAME : LETTER (LETTER | DIGIT)*
// INTEGER: DIGIT+
// var_name : NAME
// const_value : INTEGER

// factor : var_name | const_value
// store NAME or INTEGER
//TODO: add value check for var_name and const_value
class Factor : public Node {
public:
    std::string value;

    Factor(const std::string &value) {
        this->value = value;
    }

    [[nodiscard]] std::string to_string() const override {
        return value;
    }

    [[nodiscard]] bool verify() const override {
        return true;
    }
};

#endif //MINISPA_NODES_H
