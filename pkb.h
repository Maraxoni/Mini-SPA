
#ifndef MINISPA_PKB_H
#define MINISPA_PKB_H

#include <algorithm>
#include <memory>
#include <vector>

#include "benchmark_tool.h"
#include <thread>
#include <future>

//#include "storage.h"

//#include "nodes.h"
#include <array>
#include <unordered_set>

#include "parser.h"

enum TNode_type : int {
    TN_PROCEDURE,
    TN_WHILE,
    TN_ASSIGN,
    TN_EXPRESSION,
    TN_FACTOR,
    TN_CALL,
    TN_IF
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

    [[nodiscard]] std::shared_ptr<TNode> get_parent() const {
        return parent;
    }

    void set_parent(const std::shared_ptr<TNode> &parent) {
        this->parent = parent;
    }

    [[nodiscard]] std::shared_ptr<TNode> get_left_sibling() const {
        return left_sibling;
    }

    void set_left_sibling(const std::shared_ptr<TNode> &left_sibling) {
        this->left_sibling = left_sibling;
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
        } else if (std::dynamic_pointer_cast<Call>(node)) {
            type = TN_CALL;
        } else if (std::dynamic_pointer_cast<IfStmt>(node)) {
            type = TN_IF;
        }
        else {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Unknown node type.");
        }
    }

    static bool can_have_stmt_list(const std::shared_ptr<TNode> &node) {
        static const std::unordered_set allowedTypes = {
           TN_IF, TN_WHILE, TN_PROCEDURE
        };

        if (allowedTypes.count(node->get_tnode_type()))
            return true;
        return false;
    }

    [[nodiscard]] std::vector<std::shared_ptr<Node>> get_stmt_list() const {
        // casting Node to see if it's subclass that has stmt_list
        if (auto procedure = std::dynamic_pointer_cast<Procedure>(node)) {
            return procedure->stmt_list;
        } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(node)) {
            return whileStmt->stmt_list;
        } else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(node)) {
            std::vector<std::shared_ptr<Node>> result;
            result.reserve( ifStmt->then_stmt_list.size() + ifStmt->else_stmt_list.size() );
            result.insert( result.end(), ifStmt->then_stmt_list.begin(), ifStmt->then_stmt_list.end() );
            result.insert( result.end(), ifStmt->else_stmt_list.begin(), ifStmt->else_stmt_list.end() );
            return result;
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
    std::shared_ptr<TNode> parent;
    std::shared_ptr<TNode> left_sibling;
    TNode_type type;
    int mCommand_no = 0;
};

class PKB {
public:

    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> parentRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> parentTRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> followsRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> followsTRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> modifiesRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> usesRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> callsRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> callsTRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> nextRelations;
    static std::shared_ptr<std::vector<std::pair<TNode, TNode>>> nextTRelations;

    //don't allow copying
    PKB(PKB const &) = delete;

    void operator=(PKB const &) = delete;

    static PKB &instance() {
        static PKB instance;
        return instance;
    }


    //getter for ast
    [[nodiscard]] std::vector<std::shared_ptr<TNode>> get_root_nodes() const {
        return root_nodes;
    }

    void initialize() {
        this->build_AST();
        this->build_pkb_relations();
    }

    static std::vector<std::shared_ptr<Node>> get_tnode_children_as_node(const std::shared_ptr<TNode> &TNode) {
        std::vector<std::shared_ptr<Node>> children;
        switch (TNode->get_tnode_type()) {
            // returning called procedure
        case TN_CALL: {
            children.push_back(std::dynamic_pointer_cast<Call>(TNode->get_node())->procedure);
            break;
        }
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
        case TN_IF: {
            children.push_back(std::make_shared<Factor>(std::dynamic_pointer_cast<IfStmt>(TNode->get_node())->var_name));
            for (const auto &node: std::dynamic_pointer_cast<IfStmt>(TNode->get_node())->then_stmt_list) {
                children.push_back(node);
            }
            for (const auto &node: std::dynamic_pointer_cast<IfStmt>(TNode->get_node())->else_stmt_list) {
                children.push_back(node);
            }
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

    // sets parent and sibling relations between TNodes
    static void set_tnode_relations(const std::map<std::string, std::shared_ptr<Procedure>>& procedure_map) {
        for (const auto& [name, proc_ptr] : procedure_map) {
            auto new_root_node = std::make_shared<TNode>(proc_ptr);

            instance().root_nodes.push_back(new_root_node);
            instance().tnode_list.push_back(new_root_node);
        }
        for (const auto& node : instance().root_nodes) {
            set_tnode_children(node, get_tnode_children_as_node(node));
        }
    }

    static void set_tnode_children(const std::shared_ptr<TNode> &parent, std::vector<std::shared_ptr<Node>> children) {
        if (children.empty()) { return; }
        else if (children.size() == 1) {
            std::shared_ptr<TNode> first_child;
            if (std::dynamic_pointer_cast<Procedure>(children[0])) {
                first_child = instance().find_node_in_roots(children[0]);
            } else {
                first_child = std::make_shared<TNode>(children[0]);
                instance().tnode_list.push_back(first_child);
            }
            parent->set_first_child(first_child);
            first_child->set_parent(parent);
            if (first_child->get_tnode_type() != TN_PROCEDURE) {
                // procedures are handled in set_tnode_relations
                set_tnode_children(first_child, get_tnode_children_as_node(first_child));
            }
        } else {
            std::shared_ptr<TNode> current_child;
            std::shared_ptr<TNode> next_child;
            if (std::dynamic_pointer_cast<Procedure>(children[0])) {
                current_child = instance().find_node_in_roots(children[0]);
            } else {
                current_child = std::make_shared<TNode>(children[0]);
            }
            parent->set_first_child(current_child);
            for (int i = 0; i < children.size() - 1; i++) { // executes loop for every child but last
                if (std::dynamic_pointer_cast<Procedure>(children[i + 1])) {
                    next_child = instance().find_node_in_roots(children[i + 1]);
                } else {
                    next_child = std::make_shared<TNode>(children[i + 1]);
                    instance().tnode_list.push_back(current_child);
                }
                current_child->set_parent(parent);
                current_child->set_right_sibling(next_child);
                next_child->set_left_sibling(current_child);
                if (current_child->get_tnode_type() != TN_PROCEDURE) {
                    // procedures are handled in set_tnode_relations
                    set_tnode_children(current_child, get_tnode_children_as_node(current_child));
                }
                current_child = next_child;
            }
            current_child->set_parent(parent);
            if (current_child->get_tnode_type() != TN_PROCEDURE) {
                // procedures are handled in set_tnode_relations
                instance().tnode_list.push_back(current_child);
                set_tnode_children(current_child, get_tnode_children_as_node(current_child));
            }
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

    static int assign_command_no(const std::vector<std::shared_ptr<TNode>> &nodes) {
        for (auto node : nodes) {
            node->set_command_no(1);
        }
        return 0;
    }

    // node relations
    static bool is_statement(const std::shared_ptr<TNode> &node) {
        static const std::unordered_set allowedTypes = {
            TN_ASSIGN, TN_WHILE, TN_CALL, TN_IF
        };

        if (allowedTypes.count(node->get_tnode_type()))
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
            } else if (child->get_tnode_type() != TN_FACTOR) {
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
        static const std::unordered_set allowedTypes = {
            TN_CALL, TN_PROCEDURE, TN_WHILE, TN_IF, TN_ASSIGN
         };

        if (allowedTypes.count(node->get_tnode_type()))
            return true;
        return false;
    }

    static bool modifies(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node2->get_tnode_type() != TN_FACTOR && node2->get_tnode_type() != TN_ASSIGN) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only factor or assignment can be modified.");
        }
        switch (node1->get_tnode_type()) {
        case TN_CALL:
        case TN_PROCEDURE:
        case TN_WHILE:
        case TN_IF: {
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
        if (node2->get_tnode_type() != TN_FACTOR && node2->get_tnode_type() != TN_ASSIGN) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only factor or assignment can be used.");
        }
        switch (node1->get_tnode_type()) {
        case TN_CALL:
        case TN_PROCEDURE: {
            for (const auto &child: get_tnode_children(node1)) {
                if (uses(child, node2)) { return true; }
            }
            return false;
        }
        case TN_IF:
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

    static bool calls(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node2->get_tnode_type() != TN_PROCEDURE) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only procedure can be called.");
        }
        static std::unordered_set allowedTypes = { TN_IF, TN_WHILE };

        if (node1->get_tnode_type() == TN_CALL && node1->get_first_child() == node2) { return true; }

        std::vector<std::shared_ptr<TNode>> children = get_tnode_children(node1);
        for (const auto &child: children) {
            if (child->get_tnode_type() == TN_CALL) {
                if (child->get_first_child() == node2) {
                    return true;
                }
            } else if (allowedTypes.count(child->get_tnode_type())) {
                for (const auto &grandChild : get_tnode_children(child)) {
                    if (calls(grandChild, node2)) { return true; }
                }
            }
        }
        return false;
    }

    static bool callsT(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node2->get_tnode_type() != TN_PROCEDURE) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Only procedure can be called.");
        }
        static std::unordered_set allowedTypes = { TN_IF, TN_WHILE };

        if (node1->get_tnode_type() == TN_CALL && node1->get_first_child() == node2) { return true; }

        std::vector<std::shared_ptr<TNode>> children = get_tnode_children(node1);
        for (const auto &child: children) {
            if (child->get_tnode_type() == TN_CALL) {
                if (child->get_first_child() == node2) {
                    return true;
                }
                else {
                    if (callsT(child->get_first_child(), node2)) { return true; }
                }
            } else if (allowedTypes.count(child->get_tnode_type())) {
                for (const auto &grandChild : get_tnode_children(child)) {
                    if (calls(grandChild, node2)) { return true; }
                }
            }
        }
        return false;
    }

    static bool next(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node1->get_tnode_type() == TN_FACTOR || node2->get_tnode_type() == TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Factors can't be used in this relationship.");
        }
        if (node1->get_tnode_type() == TN_EXPRESSION || node2->get_tnode_type() == TN_EXPRESSION) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expressions can't be used in this relationship.");
        }
        switch (node1->get_tnode_type()) {
        case TN_PROCEDURE: {
            if (node1->get_first_child() == node2) {
                return true;
            }
            return false;
        }
        case TN_CALL: {
            if (next(node1->get_first_child(), node2)) {
                return true;
            }
            return false;
        }
        // in IF TNode children there is no distinction between then and else stmt, we have to use Nodes instead
        case TN_IF: {
            if (std::dynamic_pointer_cast<IfStmt>(node1->get_node())->then_stmt_list[0] == node2->get_node() ||
                   std::dynamic_pointer_cast<IfStmt>(node1->get_node())->else_stmt_list[0] == node2->get_node()) {
                return true;
                   }
            return false;
        }
        case TN_WHILE: {
            if (node1->get_first_child()->get_right_sibling() == node2) {
                return true;
            }
            return false;
        }
        case TN_ASSIGN: {
            if (node1->get_right_sibling()) {
                if (node1->get_right_sibling() == node2) {
                    return true;
                }
                return false;
            }
            // node is last statement in the list
            auto tmpTNode = node1->get_parent()->get_right_sibling();
            if (tmpTNode && tmpTNode == node2) {
                return true;
            }
            return false;
        }
        }
        return false;
    }

    static bool nextT(const std::shared_ptr<TNode> &node1, const std::shared_ptr<TNode> &node2) {
        if (node1->get_tnode_type() == TN_FACTOR || node2->get_tnode_type() == TN_FACTOR) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Factors can't be used in this relationship.");
        }
        if (node1->get_tnode_type() == TN_EXPRESSION || node2->get_tnode_type() == TN_EXPRESSION) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Expressions can't be used in this relationship.");
        }

        bool result = false;
        switch (node1->get_tnode_type()) {
        case TN_PROCEDURE:
            if (node1->get_first_child() == node2) {
                return true;
            } else if (node1->get_first_child()) {
                result = nextT(node1->get_first_child(), node2) ? true : result;
            }
            break;
        case TN_CALL:
            if (nextT(node1->get_first_child(), node2)) {
                return true;
            }
            break;
            // in IF TNode children there is no distinction between then and else stmt, we have to use Nodes instead
        case TN_IF:
            if (std::dynamic_pointer_cast<IfStmt>(node1->get_node())->then_stmt_list[0] == node2->get_node() ||
                std::dynamic_pointer_cast<IfStmt>(node1->get_node())->else_stmt_list[0] == node2->get_node()) {
                return true;
                } else if (node1->get_first_child()->get_right_sibling()) {
                    result = nextT(node1->get_first_child()->get_right_sibling(), node2) ? true : result;
                }
            break;
        case TN_WHILE:
            if (node1->get_first_child()->get_right_sibling() == node2) {
                return true;
            } else if (node1->get_first_child()->get_right_sibling()) {
                result = nextT(node1->get_first_child()->get_right_sibling(), node2) ? true : result;
            }
            break;
        case TN_ASSIGN:
            if (node1->get_right_sibling()) {
                if (node1->get_right_sibling() == node2) {
                    return true;
                } else {
                    result = nextT(node1->get_right_sibling(), node2) ? true : result;
                }
            } else {
                // node is last statement in the list
                auto psTNode = node1->get_parent()->get_right_sibling();
                if (psTNode) {
                    if (psTNode == node2) {
                        return true;
                    } else {
                        result = nextT(psTNode, node2) ? true : result;
                    }
                }
            }
        }
        return result;
    }

private:
    std::vector<std::shared_ptr<TNode>> root_nodes{}; // rootNode for each procedure
    std::vector<std::shared_ptr<TNode>> tnode_list{};

    PKB() = default;

    void build_pkb_relations() const {
        static std::unordered_set allowedTypes1 = { TN_IF };
        static std::unordered_set allowedTypes2 = { TN_IF };

        for (const auto &node1: tnode_list) {
            for (const auto &node2 : tnode_list) {
                if (node1 == node2) { continue; }
                allowedTypes1 = { TN_PROCEDURE, TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                allowedTypes2 = { TN_PROCEDURE, TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    parent(node1, node2)) {
                    parentRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_PROCEDURE, TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                allowedTypes2 = { TN_PROCEDURE, TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    parentT(node1, node2)) {
                    parentTRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                allowedTypes2 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    follows(node1, node2)) {
                    followsRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                allowedTypes2 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    followsT(node1, node2)) {
                    followsTRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL, TN_PROCEDURE };
                allowedTypes2 = { TN_FACTOR, TN_ASSIGN };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    modifies(node1, node2)) {
                    modifiesRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_ASSIGN, TN_CALL, TN_PROCEDURE };
                allowedTypes2 = { TN_FACTOR, TN_ASSIGN };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    uses(node1, node2)) {
                    usesRelations->emplace_back(*node1, *node2);
                    }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE };
                allowedTypes2 = { TN_PROCEDURE };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    calls(node1, node2)) {
                    callsRelations->emplace_back(*node1, *node2);
                }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE };
                allowedTypes2 = { TN_PROCEDURE };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    callsT(node1, node2)) {
                    callsTRelations->emplace_back(*node1, *node2);
                }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE, TN_ASSIGN };
                allowedTypes2 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE, TN_ASSIGN };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    next(node1, node2)) {
                    nextRelations->emplace_back(*node1, *node2);
                }
                allowedTypes1 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE, TN_ASSIGN };
                allowedTypes2 = { TN_WHILE, TN_IF, TN_CALL, TN_PROCEDURE, TN_ASSIGN };
                if (allowedTypes1.count(node1->get_tnode_type()) &&
                    allowedTypes2.count(node2->get_tnode_type()) &&
                    nextT(node1, node2)) {
                    nextTRelations->emplace_back(*node1, *node2);
                }
            }
        }
    }

    void build_AST() {
        if (!Parser::instance().initialized) {
            fatal_error(__PRETTY_FUNCTION__, __LINE__, "Parser is not initialized.");
            return;
        }

        auto procedures_map = Parser::instance().get_all_procedures();
        set_tnode_relations(procedures_map);
    }

    std::shared_ptr<TNode> find_node_in_roots(const std::shared_ptr<Node>& node) {
        for (auto tnode : root_nodes) {
            if (tnode->get_node() == node) {
                return tnode;
            }
        }

        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Node not found.");
        return nullptr;
    }
};

namespace pkb {
    void test();
}

#endif
