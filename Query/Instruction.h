// Instruction.h
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <iostream>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>

#include "SubInstruction.h"
#include "../pkb.h"

class TNode;

namespace query {
    class Instruction {
    public:
        std::vector<std::string> select_variables;
        std::vector<SubInstruction> sub_instructions;
        std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > variableResults;
        std::unordered_map<std::string, std::string> synonymTypes;

        Instruction(const std::vector<std::string> &selects) : select_variables(selects) {
        }

        void add_sub_instruction(const SubInstruction &sub) {
            sub_instructions.push_back(sub);
        }

        SubInstruction &get_last_sub_instruction() {
            if (sub_instructions.empty()) throw std::out_of_range("No sub-instructions.");
            return sub_instructions.back();
        }

        std::string get_variables_with_types_string() const {
            std::unordered_map<std::string, std::vector<std::string>> type_to_synonyms;

            for (const auto& [syn, type] : synonymTypes) {
                type_to_synonyms[type].push_back(syn);
            }

            std::ostringstream oss;
            for (const auto& [type, syn_list] : type_to_synonyms) {
                oss << type << " ";
                for (size_t i = 0; i < syn_list.size(); ++i) {
                    oss << syn_list[i];
                    if (i < syn_list.size() - 1) oss << ", ";
                }
                oss << "; ";
            }
            oss << "\n";
            return oss.str();
        }

        std::string get_instruction_string() const {
            std::ostringstream oss;

            // Sekcja SELECT
            oss << "Select ";
            if (select_variables.size() == 1 && select_variables[0] == "BOOLEAN") {
                oss << "BOOLEAN";
            } else {
                for (size_t i = 0; i < select_variables.size(); ++i) {
                    oss << select_variables[i];
                    if (i != select_variables.size() - 1) oss << ", ";
                }
            }

            // Sekcja SUCH THAT
            if (!sub_instructions.empty()) {
                oss << " such that ";

                for (size_t i = 0; i < sub_instructions.size(); ++i) {
                    const auto &sub = sub_instructions[i];
                    if (i != 0) oss << " and ";

                    oss << sub.relation << "(" << sub.left_param << ", " << sub.right_param << ")";

                    // Sekcja WITH (jeśli są)
                    if (!sub.synonym_constraints.empty()) {
                        oss << " with ";
                        for (size_t j = 0; j < sub.synonym_constraints.size(); ++j) {
                            const auto &c = sub.synonym_constraints[j];
                            oss << c.synonym << "." << c.attribute << " = \"" << c.value << "\"";
                            if (j != sub.synonym_constraints.size() - 1) oss << " and ";
                        }
                    }
                }
            }

            oss << "\n";
            return oss.str();
        }


        void process_query() {
            std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > result_set = {{}};

            std::sort(sub_instructions.begin(), sub_instructions.end(),
                      [](const SubInstruction &a, const SubInstruction &b) {
                          return instruction_weight(a) > instruction_weight(b);
                      });

            for (auto &sub: sub_instructions) {
                std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > current_result;
                auto relationData = get_relation_data(sub.relation);

                for (auto &[left, right]: relationData) {
                    std::unordered_map<std::string, std::shared_ptr<TNode> > row;

                    if (!query_is_number(sub.left_param)) {
                        row[sub.left_param] = std::make_shared<TNode>(left);
                        synonymTypes[sub.left_param] = tnode_type_to_string(left.get_tnode_type());
                    }
                    if (!query_is_number(sub.right_param)) {
                        row[sub.right_param] = std::make_shared<TNode>(right);
                        synonymTypes[sub.right_param] = tnode_type_to_string(right.get_tnode_type());
                    }

                    current_result.push_back(std::move(row));
                }

                filter_results_by_constraints(current_result, sub.synonym_constraints);
                result_set = join_results(result_set, current_result);
            }

            variableResults = std::move(result_set);
        }

        std::string get_result_string() const {
            if (select_variables.size() == 1 && select_variables[0] == "BOOLEAN") {
                return variableResults.empty() ? "false" : "true";
            }

            std::set<int> unique_values;

            // Zbierz wszystkie unikalne wartości mLineNumber ze wszystkich wyników i wszystkich synonimów
            for (const auto &row: variableResults) {
                for (const auto &var: select_variables) {
                    auto it = row.find(var);
                    if (it != row.end()) {
                        unique_values.insert(it->second->get_node()->mLineNumber);
                    }
                }
            }

            // Połącz unikalne wartości przecinkami
            std::ostringstream oss;
            size_t count = 0;
            for (int val: unique_values) {
                oss << val;
                if (++count < unique_values.size()) oss << ", ";
            }

            return oss.str();
        }


        static bool query_is_number(const std::string &str) {
            return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
        }

        static std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > >
        join_results(const std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > &existing,
                     const std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > &incoming) {
            std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > result;

            for (const auto &e: existing) {
                for (const auto &i: incoming) {
                    bool match = true;
                    auto merged = e;
                    for (const auto &[k, v]: i) {
                        if (merged.count(k) && merged[k]->get_node()->mLineNumber != v->get_node()->mLineNumber) {
                            match = false;
                            break;
                        }
                        merged[k] = v;
                    }
                    if (match) result.push_back(std::move(merged));
                }
            }
            return result;
        }

        static void filter_results_by_constraints(
            std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > &results,
            const std::vector<SynonymConstraint> &constraints) {
            if (constraints.empty()) return;

            std::vector<std::unordered_map<std::string, std::shared_ptr<TNode> > > filtered;

            for (const auto &row: results) {
                bool allMatch = true;
                for (const auto &constraint: constraints) {
                    auto it = row.find(constraint.synonym);
                    if (it == row.end()) {
                        allMatch = false;
                        break;
                    }

                    const auto &tnode = it->second;
                    if (constraint.attribute == "procName" && tnode->to_string() != constraint.value) {
                        allMatch = false;
                        break;
                    }
                    if (constraint.attribute == "stmt#" && tnode->get_node()->mLineNumber !=
                        std::stoi(constraint.value)) {
                        allMatch = false;
                        break;
                    }
                }
                if (allMatch) filtered.push_back(row);
            }

            results = std::move(filtered);
        }

    private:
        static std::string tnode_type_to_string(int tnode_type) {
            switch (tnode_type) {
                case TN_ASSIGN: return "stmt";
                case TN_WHILE: return "while";
                case TN_IF: return "stmt";
                case TN_CALL: return "call";
                case TN_FACTOR: return "variable";
                case TN_PROCEDURE: return "procedure";
                default: return "stmt";
            }
        }

        static std::vector<std::pair<TNode, TNode> > &get_relation_data(const std::string &rel) {
            if (rel == "Uses") return *PKB::usesRelations;
            if (rel == "Modifies") return *PKB::modifiesRelations;
            if (rel == "Follows") return *PKB::followsRelations;
            if (rel == "Follows*") return *PKB::followsTRelations;
            if (rel == "Parent") return *PKB::parentRelations;
            if (rel == "Parent*") return *PKB::parentTRelations;
            if (rel == "Calls") return *PKB::callsRelations;
            if (rel == "CallsT") return *PKB::callsTRelations;
            if (rel == "Next") return *PKB::nextRelations;
            if (rel == "Next*") return *PKB::nextTRelations;
            static std::vector<std::pair<TNode, TNode> > empty;
            return empty;
        }

        static int instruction_weight(const SubInstruction &sub) {
            int weight = 0;
            if (query_is_number(sub.left_param)) weight += 2;
            if (query_is_number(sub.right_param)) weight += 2;
            if (!sub.synonym_constraints.empty()) weight += 3;
            static const std::unordered_map<std::string, int> rel_weights = {
                {"Follows", 2}, {"Follows*", 1},
                {"Parent", 2}, {"Parent*", 1},
                {"Modifies", 3}, {"Uses", 3},
                {"Calls", 2}, {"CallsT", 1},
                {"Next", 2}, {"Next*", 1}
            };
            auto it = rel_weights.find(sub.relation);
            if (it != rel_weights.end()) weight += it->second;
            return weight;
        }
    };
} // namespace query

#endif // INSTRUCTION_H
