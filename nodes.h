//
// Created by Kaspek on 01.04.2025.
//

#ifndef MINISPA_NODES_H
#define MINISPA_NODES_H

#include <string>
#include <vector>
#include <memory>

// TODO: maybe add verify() to all functions, to ensure parsed node is correct
class Node {
public:
    virtual ~Node() = default;

    [[nodiscard]] virtual std::string to_string() const = 0;
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
        for (const auto &stmt : stmt_list) {
            result +=  stmt->to_string() + "\n";
        }
        result += "}\n===== END PROCEDURE " + name;
        return result;
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
        for (const auto &stmt : stmt_list) {
            result += stmt->to_string() + "\n";
        }
        result += "}\n=== END WHILE " + var_name;
        return result;
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
};

#endif //MINISPA_NODES_H
