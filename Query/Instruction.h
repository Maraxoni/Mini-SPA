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

        std::unordered_map<std::string, std::unordered_set<int>> variableResults;

        Instruction(const std::string &sel) : select(sel) {
        }

        void add_sub_instruction(const SubInstruction &sub) {
            sub_instructions.push_back(sub);
        }

        SubInstruction& get_last_sub_instruction() {
            if (sub_instructions.empty()) {
                throw std::out_of_range("No sub-instructions available.");
            }
            return sub_instructions.back();
        }

        void print_instruction() const {
            std::cout << "Select " << select << " such that ";
            bool first_sub_i = true;

            for (const auto &sub : sub_instructions) {
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

        void print_final_results_to_file(std::ofstream &processQueryFile) {
            print_instruction_to_file(processQueryFile);

        }

        void print_subquery_results_to_file(std::ofstream &processQueryFile) {
            print_instruction_to_file(processQueryFile);

        }

        void print_relations() {
            std::cout << "Follows" << std::endl;
            for (const auto& [left, right] : *PKB::followsRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
            std::cout << "Follows*" << std::endl;
            for (const auto& [left, right] : *PKB::followsTRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
            std::cout << "Parent" << std::endl;
            for (const auto& [left, right] : *PKB::parentRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
            std::cout << "Parent*" << std::endl;
            for (const auto& [left, right] : *PKB::parentTRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
            std::cout << "Modifies" << std::endl;
            for (const auto& [left, right] : *PKB::modifiesRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
            std::cout << "Uses" << std::endl;
            for (const auto& [left, right] : *PKB::usesRelations) {
                std::cout << left.get_command_no() << ": " << right.get_command_no() <<", ";
            }
        }

        void process_query() {
            std::cout << "Searching parsed code for: " << select << std::endl;

            for (const SubInstruction &sub : sub_instructions) {
                std::shared_ptr<std::vector<std::pair<TNode, TNode>>> relationVec;

                if (sub.relation == "Follows") {
                    std::cout << "Checking Follows relation\n";
                    relationVec = PKB::followsRelations;
                } else if (sub.relation == "Follows*") {
                    std::cout << "Checking Follows* relation\n";
                    relationVec = PKB::followsTRelations;
                } else if (sub.relation == "Parent") {
                    std::cout << "Checking Parent relation\n";
                    relationVec = PKB::parentRelations;
                } else if (sub.relation == "Parent*") {
                    std::cout << "Checking Parent* relation\n";
                    relationVec = PKB::parentTRelations;
                } else if (sub.relation == "Modifies") {
                    std::cout << "Checking Modifies relation\n";
                    relationVec = PKB::modifiesRelations;
                } else if (sub.relation == "Uses") {
                    std::cout << "Checking Uses relation\n";
                    relationVec = PKB::usesRelations;
                } else {
                    std::cout << "Unknown relation type: " << sub.relation << "\n";
                    continue;
                }

                // for (true) {
                //     _sleep(1);
                // }

            }
        }


        bool query_is_number(const std::string &str) {
            return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
        }
    };
} // query

#endif //INSTRUCTION_H
