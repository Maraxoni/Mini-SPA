#ifndef MINISPA_PKB_H
#define MINISPA_PKB_H

#include <memory>
#include <vector>

#include "nodes.h"
#include "parser.h"

enum TNodeType : int {
  TN_PROCEDURE,
  TN_WHILE,
  TN_ASSIGN,
  TN_EXPRESSION,
  TN_FACTOR
};

class TNode {
public:
    TNode(std::shared_ptr<Node>node) {
        this->node = std::move(node);
        this->set_node_type();
    }

    std::string to_string()
    {
        return node->to_string();
    }

    std::shared_ptr<Node> get_node() {
      return node;
    }

    void set_first_child(std::shared_ptr<TNode> child) {
      this->firstChild = child;
    }

    std::shared_ptr<TNode> get_first_child() {
        return firstChild;
    }

    void set_right_sibling(std::shared_ptr<TNode> sibling) {
        this->rightSibling = sibling;
    }

    std::shared_ptr<TNode> get_right_sibling() {
        return rightSibling;
    }

    void set_node_type(){
        if (std::dynamic_pointer_cast<Procedure>(node)) {
            type = TN_PROCEDURE;
        } else if (std::dynamic_pointer_cast<WhileStmt>(node)) {
            type = TN_WHILE;
        } else if (std::dynamic_pointer_cast<Assign>(node)) {
            type = TN_ASSIGN;
        } else if (std::dynamic_pointer_cast<Expr>(node)) {
            type = TN_EXPRESSION;
        } else if (std::dynamic_pointer_cast<Factor>(node)) {
            type = TN_FACTOR;
        }
    }

    TNodeType get_node_type(){
      return type;
    }

    std::vector<std::shared_ptr<Node>> get_stmt_list() {
        // casting Node to see if it's subclass that has stmt_list
        if (auto procedure = std::dynamic_pointer_cast<Procedure>(node)) {
            return procedure->stmt_list;
        } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(node)) {
            return whileStmt->stmt_list;
        }
        else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Tried getting statement list from node which doesn't have one.");
            return {};
        }
    }


private:
    std::shared_ptr<Node> node;
    std::shared_ptr<TNode> firstChild;
    std::shared_ptr<TNode> rightSibling;
    TNodeType type;
};

class PKB {
public:
    explicit PKB(std::shared_ptr<Parser> parser) {
        this->parser = std::move(parser);
    }

    std::vector<std::shared_ptr<Node>> get_TNode_children(std::shared_ptr<TNode> TNode) {
        std::vector<std::shared_ptr<Node>> children;
        switch (TNode->get_node_type()) {
            // returning statement list
            case TN_PROCEDURE: {
                children = std::dynamic_pointer_cast<Procedure>(TNode->get_node())->stmt_list;
                break;
            }
            // returning conditional variable and statement list
            case TN_WHILE: {
                children.push_back(std::make_shared<Factor>(std::dynamic_pointer_cast<WhileStmt>(TNode->get_node())->var_name));
                for (std::shared_ptr<Node> node : std::dynamic_pointer_cast<WhileStmt>(TNode->get_node())->stmt_list) {
                    children.push_back(node);
                }
                break;
            }
            // returning variable and expression
            case TN_ASSIGN: {
                children.push_back(std::make_shared<Factor>(std::dynamic_pointer_cast<Assign>(TNode->get_node())->var_name));
                children.push_back(std::dynamic_pointer_cast<Assign>(TNode->get_node())->expr);
                break;
            }
            // returning left and right piece of expression
            case TN_EXPRESSION: {
                children.push_back(std::dynamic_pointer_cast<Expr>(TNode->get_node())->left);
                children.push_back(std::dynamic_pointer_cast<Expr>(TNode->get_node())->right);
            break;
            }
            // factor can't have children, returning empty list
            case TN_FACTOR: {
                break;
            }
        }

        return children;
    }

    void set_TNode_children(std::shared_ptr<TNode> parent, std::vector<std::shared_ptr<Node>> children) {
        if (children.size() == 0) { return; }
        else if (children.size() == 1) {
            std::shared_ptr<TNode> first_child = std::make_shared<TNode>(children[0]);
            parent->set_first_child(first_child);
            set_TNode_children(first_child, get_TNode_children(first_child));
        }
        else {
            std::shared_ptr<TNode> current_child = std::make_shared<TNode>(children[0]);
            std::shared_ptr<TNode> next_child;
            parent->set_first_child(current_child);
            for (int i = 0; i < children.size() - 1; i++) {
                next_child = std::make_shared<TNode>(children[i + 1]);
                current_child->set_right_sibling(next_child);
                set_TNode_children(current_child, get_TNode_children(current_child));
                current_child = next_child;
            }
            set_TNode_children(current_child, get_TNode_children(current_child));
        }
    }

    std::shared_ptr<TNode> build_AST() {
        if (!parser->initialized) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Parser is not initialized.");
            return nullptr;
        }

        auto rootNode = std::make_shared<TNode>(parser->parse_procedure());
        set_TNode_children(rootNode, get_TNode_children(rootNode));

        return rootNode;
    }

    // not necessary for now, maybe will be useful later
    // const std::vector<std::shared_ptr<Procedure>>& get_procedure_list() const {
    //     return procedure_list;
    // }
    //
    // void store_procedure(std::shared_ptr<Procedure> procedure) {
    //     procedure_list.push_back(std::move(procedure));
    // }

private:
    std::shared_ptr<Parser> parser;
    // std::vector<std::shared_ptr<Procedure>> procedure_list;
};

namespace pkb {
    void test();
}

#endif
