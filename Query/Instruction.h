#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <algorithm>

#include "SubInstruction.h"
#include "../pkb.h"

class TNode;

namespace query {
    class Instruction {
    public:
        std::vector<std::string> select_variables;
        std::vector<SubInstruction> sub_instructions;
        std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > variable_result;
        std::unordered_map<std::string, std::string> variable_types;
        bool same_values_cond = false;

        Instruction(const std::vector<std::string> &selects) : select_variables(selects) {
        }

        void add_sub_instruction(const SubInstruction &sub) {
            sub_instructions.push_back(sub);
        }

        std::string get_variables_with_types_string() const {
            // Map type to list of variables
            std::unordered_map<std::string, std::vector<std::string> > type_to_vars;
            for (const auto &[var, type]: variable_types) {
                type_to_vars[type].push_back(var);
            }

            // Sort types alphabetically
            std::vector<std::string> sorted_types;
            for (const auto &[type, _]: type_to_vars) {
                sorted_types.push_back(type);
            }
            std::sort(sorted_types.begin(), sorted_types.end());

            std::ostringstream oss;
            for (size_t i = 0; i < sorted_types.size(); ++i) {
                const std::string &type = sorted_types[i];
                auto vars = type_to_vars.at(type);
                std::sort(vars.begin(), vars.end());

                oss << type << " ";
                for (size_t j = 0; j < vars.size(); ++j) {
                    oss << vars[j];
                    if (j + 1 < vars.size()) {
                        oss << ", ";
                    }
                }
                oss << ";";
                if (i + 1 < sorted_types.size()) {
                    oss << " ";
                }
            }

            oss << "\n";

            return oss.str();
        }


        std::string get_instruction_string() const {
            std::ostringstream oss;

            // Select clause
            oss << "Select ";
            if (select_variables.size() == 1) {
                oss << select_variables[0];
            } else {
                oss << "<";
                for (size_t i = 0; i < select_variables.size(); ++i) {
                    oss << select_variables[i];
                    if (i + 1 < select_variables.size()) oss << ", ";
                }
                oss << ">";
            }

            if (!sub_instructions.empty()) {
                oss << " such that ";

                for (size_t i = 0; i < sub_instructions.size(); ++i) {
                    const auto &s = sub_instructions[i];

                    // Relation
                    oss << s.relation << "(" << s.left_param << ", " << s.right_param << ") ";

                    // If there are WITH constraints, print them right after the relation
                    if (!s.synonym_constraints.empty()) {
                        for (size_t j = 0; j < s.synonym_constraints.size(); ++j) {
                            const auto &c = s.synonym_constraints[j];
                            oss << "with " << c.synonym << "." << c.attribute << " = " << c.value << " ";
                        }
                    }

                    // If this is not the last sub_instruction, join with "and"
                    if (i + 1 < sub_instructions.size()) {
                        oss << "and ";
                    }
                }
            }

            oss << "\n";

            return oss.str();
        }

        void process_query() {
            using Binding = std::unordered_map<std::string, std::shared_ptr<TNode> >;
            std::vector<Binding> partialResults = {{}};

            // Sort clauses to start with the most restrictive ones
            std::vector<SubInstruction> sorted_subs = sub_instructions;
            std::sort(sorted_subs.begin(), sorted_subs.end(), [](const SubInstruction &a, const SubInstruction &b) {
                return get_relation_data(a.relation).size() < get_relation_data(b.relation).size();
            });

            for (const auto &sub: sorted_subs) {
                const auto &rel_data = get_relation_data(sub.relation);
                std::vector<Binding> newResults;

                for (const auto &[left, right]: rel_data) {
                    for (const auto &binding: partialResults) {
                        Binding newBinding = binding;
                        bool valid = true;

                        // Handling left parameter
                        if (is_variable(sub.left_param)) {
                            if (newBinding.count(sub.left_param)) {
                                if (newBinding[sub.left_param]->get_node()->mLineNumber != left.get_node()->
                                    mLineNumber) {
                                    continue; // conflict, skip
                                }
                            } else {
                                newBinding[sub.left_param] = std::make_shared<TNode>(left);
                            }
                        } else {
                            if (std::to_string(left.get_node()->mLineNumber) != sub.left_param) {
                                continue; // literal doesn't match
                            }
                        }

                        // Handling right parameter
                        if (is_variable(sub.right_param)) {
                            if (newBinding.count(sub.right_param)) {
                                if (newBinding[sub.right_param]->get_node()->mLineNumber != right.get_node()->
                                    mLineNumber) {
                                    continue;
                                }
                            } else {
                                newBinding[sub.right_param] = std::make_shared<TNode>(right);
                            }
                        } else {
                            if (std::to_string(right.get_node()->mLineNumber) != sub.right_param) {
                                continue;
                            }
                        }

                        // Handling WITH constraints
                        for (const auto &syn_constraint: sub.synonym_constraints) {
                            const std::string &syn = syn_constraint.synonym;
                            const std::string &attr = syn_constraint.attribute;
                            const std::string &val = syn_constraint.value;

                            if (!newBinding.count(syn)) {
                                valid = false;
                                break;
                            }

                            auto node = newBinding[syn]->get_node();

                            if (is_variable(val)) {
                                if (!newBinding.count(val)) {
                                    valid = false;
                                    break;
                                }

                                auto val_node = newBinding[val]->get_node();
                                if (attr == "stmt#" || attr == "value") {
                                    if (node->mLineNumber != val_node->mLineNumber) {
                                        valid = false;
                                        break;
                                    }
                                } else if (attr == "procName" || attr == "varName") {
                                    if (node->mLineNumber != val_node->mLineNumber) {
                                        valid = false;
                                        break;
                                    }
                                } else {
                                    valid = false;
                                    break;
                                }
                            } else {
                                // Literal value
                                if (attr == "stmt#" || attr == "value") {
                                    if (std::to_string(node->mLineNumber) != val) {
                                        valid = false;
                                        break;
                                    }
                                } else if (attr == "procName" || attr == "varName") {
                                    if (std::to_string(node->mLineNumber) != val) {
                                        valid = false;
                                        break;
                                    }
                                } else {
                                    valid = false;
                                    break;
                                }
                            }
                        }

                        if (valid) {
                            newResults.push_back(newBinding);
                        }
                    }
                }

                partialResults = std::move(newResults);
            }

            // Save final results
            variable_result = partialResults;
        }


        std::string get_result_string() const {
            if ((select_variables.size() == 1 || same_values_cond == true) && select_variables[0] == "BOOLEAN") {
                return variable_result.empty() ? "false" : "true";
            }

            // Set to detect unique results
            std::set<std::string> unique_result_strings;
            std::vector<std::pair<std::string, std::vector<int> > > sortable_results;

            for (const auto &result_per_variable: variable_result) {
                std::ostringstream result_line;
                std::vector<int> numeric_values;

                for (size_t i = 0; i < select_variables.size(); ++i) {
                    const std::string &select = select_variables[i];
                    auto it = result_per_variable.find(select);

                    if (i > 0) result_line << " ";

                    if (it != result_per_variable.end() && it->second) {
                        auto node = it->second;
                        auto syn_type_it = variable_types.find(select);

                        if (syn_type_it != variable_types.end() && syn_type_it->second == "variable") {
                            result_line << node->to_string();
                            numeric_values.push_back(0); // treat as 0 for sorting
                        } else if (
                            syn_type_it != variable_types.end() && syn_type_it->second == "procedure"
                        ) {
                            result_line << node->get_node()->mLineNumber;
                            numeric_values.push_back(0); // treat as 0 for sorting
                        } else {
                            int line = node->get_node()->mLineNumber;
                            result_line << line;
                            numeric_values.push_back(line);
                        }
                    } else {
                        result_line << "null";
                        numeric_values.push_back(-1); // no data
                    }
                }

                std::string final_str = result_line.str();
                if (unique_result_strings.insert(final_str).second) {
                    // if string was unique (insert returned true), add to sortable results
                    sortable_results.emplace_back(final_str, numeric_values);
                }
            }

            if (sortable_results.empty()) {
                return "none";
            }

            std::sort(sortable_results.begin(), sortable_results.end(),
                      [](const auto &a, const auto &b) {
                          return a.second < b.second;
                      });

            std::ostringstream oss;
            for (size_t i = 0; i < sortable_results.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << sortable_results[i].first;
            }

            return oss.str();
        }

    private:
        static bool is_variable(const std::string &param) {
            return !param.empty() && isalpha(param[0]) && param != "BOOLEAN";
        }

        static const std::vector<std::pair<TNode, TNode> > &get_relation_data(const std::string &rel) {
            if (rel == "Uses") return *PKB::usesRelations;
            if (rel == "Modifies") return *PKB::modifiesRelations;
            if (rel == "Follows") return *PKB::followsRelations;
            if (rel == "Follows*") return *PKB::followsTRelations;
            if (rel == "Parent") return *PKB::parentRelations;
            if (rel == "Parent*") return *PKB::parentTRelations;
            if (rel == "Calls") return *PKB::callsRelations;
            if (rel == "Calls*") return *PKB::callsTRelations;
            if (rel == "Next") return *PKB::nextRelations;
            if (rel == "Next*") return *PKB::nextTRelations;

            static const std::vector<std::pair<TNode, TNode> > empty;
            return empty;
        }
    };
} // namespace query

#endif // INSTRUCTION_H
