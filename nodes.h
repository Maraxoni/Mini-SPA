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
    virtual void print(int indent = 0) const = 0;
    void print_indent(int indent) const {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
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
    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Procedure: " << name << "\n";
        for (const auto& stmt : stmt_list) {
            stmt->print(indent + 1);
        }
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

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "While: " << var_name << "\n";
        for (const auto& stmt : stmt_list) {
            stmt->print(indent + 1);
        }
    }



};

class IfStmt : public Node {
public:
    std::string var_name;
    std::vector<std::shared_ptr<Node>> then_stmt_list;
    std::vector<std::shared_ptr<Node>> else_stmt_list;

    IfStmt(const std::string& var_name,
           const std::vector<std::shared_ptr<Node>>& then_stmt_list,
           const std::vector<std::shared_ptr<Node>>& else_stmt_list)
            : var_name(var_name), then_stmt_list(then_stmt_list), else_stmt_list(else_stmt_list) {}

    [[nodiscard]] std::string to_string() const override {
        std::string result = "=== if " + var_name + " then {\n";
        for (const auto& stmt : then_stmt_list) {
            result += stmt->to_string() + "\n";
        }
        result += "} end if === else {\n";
        for (const auto& stmt : else_stmt_list) {
            result += stmt->to_string() + "\n";
        }
        result += "} === end else";
        return result;
    }

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "If: " << var_name << "\n";
        print_indent(indent);
        std::cout << "Then:\n";
        for (const auto& stmt : then_stmt_list) {
            stmt->print(indent + 1);
        }
        print_indent(indent);
        std::cout << "Else:\n";
        for (const auto& stmt : else_stmt_list) {
            stmt->print(indent + 1);
        }
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

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Assign: " << var_name << "\n";
        expr->print(indent + 1);
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

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Expr: " << op << "\n";
        left->print(indent + 1);  // recur left
        right->print(indent + 1);  // recur right
    }


};

// NAME : LETTER (LETTER | DIGIT)*
// INTEGER: DIGIT+
// var_name : NAME
// const_value : INTEGER

// factor : var_name | const_value
// store NAME or INTEGER
class Factor : public Node {
public:
    std::string value;

    explicit Factor(const std::string &value) {
        this->value = value;
    }

    [[nodiscard]] std::string get_value() const {
        return value;
    }

    [[nodiscard]] std::string to_string() const override {
        return value;
    }

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Factor: " << value << "\n";
    }


};

// call  atm print
// TODO: recurencje?

class Call : public Node {
public:
    std::string proc_name;

    explicit Call(const std::string &proc_name) : proc_name(proc_name) {}

    [[nodiscard]]  std::string to_string() const override {
        return "call " + proc_name + ";";
    }

    void print(int indent = 0) const override {
        print_indent(indent);
        std::cout << "Call: " << proc_name << "\n";
    }
};

#endif //MINISPA_NODES_H
