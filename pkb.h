#ifndef MINISPA_PKB_H
#define MINISPA_PKB_H

#include <algorithm>
#include <memory>
#include <vector>

//#include "storage.h"

//#include "nodes.h"
#include "parser.h"

enum TNode_type : int {
    TN_PROCEDURE,
    TN_WHILE,
    TN_ASSIGN,
    TN_EXPRESSION,
    TN_FACTOR
};

class TNode {
public:
    explicit TNode(std::shared_ptr<Node> node) {
        this->node = std::move(node);
        this->set_tnode_type();
    }

    [[nodiscard]] std::string to_string() const {
        return node->to_string();
    }

    [[nodiscard]] std::shared_ptr<Node> get_node() const {
        return node;
    }

    [[nodiscard]] int get_command_no() const {
        return mCommand_no;
    }

    void set_command_no(const int command_no) {
        this->mCommand_no = command_no;
    }

    [[nodiscard]] std::shared_ptr<TNode> get_first_child() const {
        return first_child;
    }

    void set_first_child(const std::shared_ptr<TNode> &child) {
        this->first_child = child;
    }

    [[nodiscard]] std::shared_ptr<TNode> get_right_sibling() const {
        return right_sibling;
    }

    void set_right_sibling(const std::shared_ptr<TNode> &sibling) {
        this->right_sibling = sibling;
    }

    [[nodiscard]] TNode_type get_tnode_type() const {
        return type;
    }

    void set_tnode_type() {
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

    [[nodiscard]] std::vector<std::shared_ptr<Node>> get_stmt_list() const {
        // casting Node to see if it's subclass that has stmt_list
        if (auto procedure = std::dynamic_pointer_cast<Procedure>(node)) {
            return procedure->stmt_list;
        } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(node)) {
            return whileStmt->stmt_list;
        } else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__,
                        "Tried getting statement list from node which doesn't have one.");
            return {};
        }
    }

private:
    std::shared_ptr<Node> node;
    std::shared_ptr<TNode> first_child;
    std::shared_ptr<TNode> right_sibling;
    TNode_type type;
    int mCommand_no;
};

class PKB {
public:
    static std::vector<std::shared_ptr<Node>> get_tnode_children_as_node(const std::shared_ptr<TNode> &TNode) {
        std::vector<std::shared_ptr<Node>> children;
        switch (TNode->get_tnode_type()) {
            // returning statement list
            case TN_PROCEDURE: {
                children = std::dynamic_pointer_cast<Procedure>(TNode->get_node())->stmt_list;
                break;
            }
                // returning conditional variable and statement list
            case TN_WHILE: {
                children.push_back(
                        std::make_shared<Factor>(std::dynamic_pointer_cast<WhileStmt>(TNode->get_node())->var_name));
                for (const auto &node: std::dynamic_pointer_cast<WhileStmt>(TNode->get_node())->stmt_list) {
                    children.push_back(node);
                }
                break;
            }
                // returning variable and expression
            case TN_ASSIGN: {
                children.push_back(
                        std::make_shared<Factor>(std::dynamic_pointer_cast<Assign>(TNode->get_node())->var_name));
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

    static std::vector<std::shared_ptr<TNode>> get_tnode_children(const std::shared_ptr<TNode> &tnode) {
        std::vector<std::shared_ptr<TNode>> children;
        if (tnode->get_first_child() != nullptr) {
            children.push_back(tnode->get_first_child());
            std::shared_ptr<TNode> current_child = tnode->get_first_child();
            while (current_child->get_right_sibling() != nullptr) {
                current_child = current_child->get_right_sibling();
                children.push_back((current_child));
            }
        }

        return children;
    }

    static void set_tnode_children(const std::shared_ptr<TNode> &parent, std::vector<std::shared_ptr<Node>> children) {
        if (children.empty()) { return; }
        else if (children.size() == 1) {
            const auto first_child = std::make_shared<TNode>(children[0]);
            parent->set_first_child(first_child);
            set_tnode_children(first_child, get_tnode_children_as_node(first_child));
        } else {
            auto current_child = std::make_shared<TNode>(children[0]);
            std::shared_ptr<TNode> next_child;
            parent->set_first_child(current_child);
            for (int i = 0; i < children.size() - 1; i++) {
                next_child = std::make_shared<TNode>(children[i + 1]);
                current_child->set_right_sibling(next_child);
                set_tnode_children(current_child, get_tnode_children_as_node(current_child));
                current_child = next_child;
            }
            set_tnode_children(current_child, get_tnode_children_as_node(current_child));
        }
    }

    static std::vector<std::shared_ptr<TNode>> get_ast_as_list(const std::shared_ptr<TNode> &rootNode,
                                                        const bool notRootFlag = false) {
        std::vector<std::shared_ptr<TNode>> result;
        if (notRootFlag == false) { result.push_back(rootNode); }
        for (const auto &child: get_tnode_children(rootNode)) {
            result.push_back(child);
            auto tmp = get_ast_as_list(child, true);
            result.insert(result.end(), tmp.begin(), tmp.end());
        }

        return result;
    }

    static int assign_command_no(const std::shared_ptr<TNode> &node, int command_no = 1) {
        node->set_command_no(command_no);
        switch (node->get_tnode_type()) {
            case TN_PROCEDURE: {
                for (const auto &child: get_tnode_children(node)) {
                    command_no++;
                    command_no = assign_command_no(child, command_no);
                }
                break;
            }
            case TN_WHILE: {
                node->get_first_child()->set_command_no(command_no);
                auto children = get_tnode_children(node);
                children.erase(children.begin()); // ignoring first child (conditional variable)
                for (const auto &child: children) {
                    command_no++;
                    command_no = assign_command_no(child, command_no);
                }
                break;
            }
            case TN_ASSIGN:
            case TN_EXPRESSION:
            case TN_FACTOR: {
                node->set_command_no(command_no);
                for (const auto &child: get_tnode_children(node)) {
                    command_no = assign_command_no(child, command_no);
                }
                break;
            }
        }

        return command_no;
    }

    [[nodiscard]] static std::shared_ptr<TNode> build_AST() {
        if (!Parser::instance().initialized) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Parser is not initialized.");
            return nullptr;
        }

        auto rootNode = std::make_shared<TNode>(Parser::instance().parse_procedure());
        set_tnode_children(rootNode, get_tnode_children_as_node(rootNode));

        assign_command_no(rootNode);

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

    // node relations
    static bool is_statement(const std::shared_ptr<TNode> &node) {
        if (node->get_tnode_type() == TN_ASSIGN || node->get_tnode_type() == TN_WHILE)
            return true;
        return false;
    }

    static bool parent(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node1->get_tnode_type() == TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Factor node can't be a parent.");
        }

        for (const auto &child: get_tnode_children(node1)) {
            if (child == node2) {
                return true;
            }
        }

        return false;
    }

    static bool parentT(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node1->get_tnode_type() == TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Factor node can't be a parent.");
        }

        bool result = false;
        for (const auto &child: get_tnode_children(node1)) {
            if (child == node2) {
                return true;
            } else {
                result = parentT(child, node2) ? true : result;
            }
        }
        return result;
    }

    static bool follows(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (!is_statement(node1)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Node1 is not a statement node.");
        } else if (!is_statement(node2)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Node2 is not a statement node.");
        }

        if (node1->get_right_sibling() == node2) {
            return true;
        } else {
            return false;
        }
    }

    static bool followsT(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (!is_statement(node1)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Node1 is not a statement node.");
        } else if (!is_statement(node2)) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Node2 is not a statement node.");
        }

        bool result = false;
        if (node1->get_right_sibling() == node2) {
            return true;
        } else if (node1->get_right_sibling() != nullptr) {
            result = followsT(node1->get_right_sibling(), node2) ? true : result;
        }
        return result;
    }

    static bool can_modify(const std::shared_ptr<TNode> &node) {
        switch (node->get_tnode_type()) {
            case TN_ASSIGN:
            case TN_WHILE:
            case TN_PROCEDURE:
                return true;
            default:
                return false;
        }
    }

    static bool modifies(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node2->get_tnode_type() != TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only factor can be modified.");
        }
        switch (node1->get_tnode_type()) {
            case TN_PROCEDURE:
            case TN_WHILE: {
                for (const auto &child: get_tnode_children(node1)) {
                    if (modifies(child, node2)) { return true; }
                }
                return false;
            }
            case TN_ASSIGN: {
                if (node1->get_first_child() == node2) {
                    return true;
                } else {
                    return false;
                }
            }
            default: {
                return false;
            }
        }
    }

    static bool uses(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node2->get_tnode_type() != TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only factor can be used.");
        }
        switch (node1->get_tnode_type()) {
            case TN_PROCEDURE: {
                for (const auto &child: get_tnode_children(node1)) {
                    if (uses(child, node2)) { return true; }
                }
                return false;
            }
            case TN_WHILE: {
                if (node1->get_first_child() == node2) { return true; }
                std::vector<std::shared_ptr<TNode>> children = get_tnode_children(node1);
                children.erase(children.begin()); // ignoring first child (conditional variable)
                for (const auto &child: get_tnode_children(node1)) {
                    if (uses(child, node2)) { return true; }
                }
                return false;
            }
            case TN_ASSIGN: {
                const std::shared_ptr<TNode> child = node1->get_first_child()->get_right_sibling();
                if (child->get_tnode_type() == TN_FACTOR) {
                    if (child == node2) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    // child is expression
                    return uses(child, node2);
                }
            }
            case TN_EXPRESSION: {
                bool result = false;
                for (const auto &child: get_tnode_children(node1)) {
                    if (child->get_tnode_type() == TN_FACTOR) {
                        if (child == node2) {
                            result = true;
                        }
                    } else {
                        // child is expression
                        result = uses(child, node2);
                    }
                }
                return result;
            }
            default: {
                return false;
            }
        }
    }

private:
//    std::shared_ptr<Parser> parser;
    // std::vector<std::shared_ptr<Procedure>> procedure_list;
};

namespace pkb {
    void test();
}

#endif
