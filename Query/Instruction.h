//
// Created by Marax on 27.04.2025.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <iostream>
#include <fstream>
#include <list>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "query.h"
#include "SubInstruction.h"
#include "../pkb.h"

class TNode;

namespace query {
    class Instruction {
    public:
        std::string select;
        std::vector<SubInstruction> sub_instructions;

        std::vector<std::unordered_map<std::string, int> > variableResults;

        Instruction(const std::string &sel) : select(sel) {
        }

        void add_sub_instruction(const SubInstruction &sub) {
            sub_instructions.push_back(sub);
        }

        SubInstruction &get_last_sub_instruction() {
            if (sub_instructions.empty()) {
                throw std::out_of_range("No sub-instructions available.");
            }
            return sub_instructions.back();
        }

        void print_instruction() const {
            std::cout << "Select " << select << " such that ";
            bool first_sub_i = true;

            for (const auto &sub: sub_instructions) {
                if (!first_sub_i) {
                    std::cout << " and ";
                } else {
                    first_sub_i = false;
                }

                std::cout << sub.relation << "(" << sub.left_param << ", " << sub.right_param << ")";

                // Multiple "with"
                for (size_t i = 0; i < sub.synonym_constraints.size(); ++i) {
                    const auto &syn = sub.synonym_constraints[i];
                    std::cout << (i == 0 ? " with " : " and ")
                            << syn.synonym << "." << syn.attribute << " = " << syn.value;
                }
            }

            std::cout << std::endl;
        }

        void print_instruction_to_file(std::ofstream &processQueryFile) const {
            processQueryFile << "Select " << select << " such that ";
            bool first_sub_i = true;
            for (const auto &sub: sub_instructions) {
                if (first_sub_i) {
                    first_sub_i = false;
                } else {
                    processQueryFile << " and ";
                }
                processQueryFile << sub.relation << " (" << sub.left_param << ", " << sub.right_param << ")";
                // Multiple "with"
                for (size_t i = 0; i < sub.synonym_constraints.size(); ++i) {
                    const auto &syn = sub.synonym_constraints[i];
                    processQueryFile << (i == 0 ? " with " : " and ")
                            << syn.synonym << "." << syn.attribute << " = " << syn.value;
                }
            }
            processQueryFile << std::endl;
        }

        void print_final_results_to_file(std::ofstream &processQueryFile) const {
            print_instruction_to_file(processQueryFile);
            processQueryFile << "Results for Select " << select << ":\n";

            std::unordered_set<int> seen;
            for (const auto &row: variableResults) {
                if (row.count(select)) {
                    seen.insert(row.at(select));
                }
            }

            for (int val: seen) {
                processQueryFile << val << ", ";
            }
            processQueryFile << "\n";
        }

        void print_distinct_results_to_file(std::ofstream &processQueryFile) {
            print_instruction_to_file(processQueryFile);
            processQueryFile << "Grouped results for Select " << select << ":\n";

            // Map
            std::unordered_map<int, std::unordered_set<int> > resultGroups;

            for (const auto &row: variableResults) {
                if (!row.count(select)) continue;
                int mainVal = row.at(select);

                for (const auto &[key, val]: row) {
                    if (key == select) continue;
                    resultGroups[mainVal].insert(val);
                }
            }

            for (const auto &[mainVal, relatedVals]: resultGroups) {
                processQueryFile << mainVal << ": ";
                bool first = true;
                for (int val: relatedVals) {
                    if (!first) processQueryFile << ", ";
                    processQueryFile << val;
                    first = false;
                }
                processQueryFile << " | ";
            }
            processQueryFile << "\n";
        }

        void print_subquery_results(std::ofstream &processQueryFile) const {
        }

        void process_query() {
            std::cout << "Searching parsed code for: " << select << std::endl;

            std::vector<std::unordered_map<std::string, int> > intermediateResults;

            for (const SubInstruction &sub: sub_instructions) {
                std::shared_ptr<std::vector<std::pair<TNode, TNode> > > relationVec;

                if (sub.relation == "Follows") {
                    relationVec = PKB::followsRelations;
                } else if (sub.relation == "Follows*") {
                    relationVec = PKB::followsTRelations;
                } else if (sub.relation == "Parent") {
                    relationVec = PKB::parentRelations;
                } else if (sub.relation == "Parent*") {
                    relationVec = PKB::parentTRelations;
                } else if (sub.relation == "Modifies") {
                    relationVec = PKB::modifiesRelations;
                } else if (sub.relation == "Uses") {
                    relationVec = PKB::usesRelations;
                } else {
                    std::cout << "Unknown relation: " << sub.relation << "\n";
                    continue;
                }

                std::vector<std::unordered_map<std::string, int> > currentRelationResults;

                for (const auto &[left, right]: *relationVec) {
                    std::unordered_map<std::string, int> row;

                    if (!query_is_number(sub.left_param)) {
                        row[sub.left_param] = left.get_command_no();
                    } else if (std::stoi(sub.left_param) != left.get_command_no()) {
                        continue; // liczba nie pasuje
                    }

                    if (!query_is_number(sub.right_param)) {
                        row[sub.right_param] = right.get_command_no();
                    } else if (std::stoi(sub.right_param) != right.get_command_no()) {
                        continue;
                    }

                    currentRelationResults.push_back(row);
                }

                // JOIN z poprzednimi wynikami
                if (intermediateResults.empty()) {
                    intermediateResults = currentRelationResults;
                } else {
                    intermediateResults = join_results(intermediateResults, currentRelationResults);
                }
            }

            variableResults = intermediateResults;
        }

        std::vector<std::unordered_map<std::string, int> >
        join_results(const std::vector<std::unordered_map<std::string, int> > &existing,
                     const std::vector<std::unordered_map<std::string, int> > &incoming) {
            std::vector<std::unordered_map<std::string, int> > result;

            for (const auto &eRow: existing) {
                for (const auto &iRow: incoming) {
                    bool match = true;
                    std::unordered_map<std::string, int> merged = eRow;

                    for (const auto &[key, val]: iRow) {
                        if (merged.count(key)) {
                            if (merged[key] != val) {
                                match = false;
                                break;
                            }
                        } else {
                            merged[key] = val;
                        }
                    }

                    if (match) {
                        result.push_back(std::move(merged));
                    }
                }
            }

            return result;
        }


        bool query_is_number(const std::string &str) {
            return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
        }
    };
} // query

#endif //INSTRUCTION_H
