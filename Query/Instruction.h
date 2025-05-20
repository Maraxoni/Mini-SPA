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

        // void print_final_results_to_file(std::ofstream &processQueryFile) {
        //     print_instruction_to_file(processQueryFile);
        //     int id = sub_instructions.size();
        //     processQueryFile << "Result: \n";
        //     SubInstruction finalInstruction = sub_instructions[id];
        //     if (select == "BOOLEAN" || select == "boolean") {
        //         if (finalInstruction.resultsMap.size() <= 0) {
        //             processQueryFile << "FALSE";
        //         } else {
        //             processQueryFile << "TRUE";
        //         }
        //     } else {
        //         for (const auto &[key, valueList]: finalInstruction.resultsMap) {
        //             processQueryFile << key->
        //                     get_command_no() <<
        //                     "";
        //             for (const std::shared_ptr<TNode> &node: valueList) {
        //                 processQueryFile << node->
        //                         get_command_no() <<
        //                         ", ";
        //             }
        //         }
        //         processQueryFile << "\n";
        //     }
        // }

        // void print_subquery_results_to_file(std::ofstream &processQueryFile) {
        //     print_instruction_to_file(processQueryFile);
        //     int subid = 1;
        //     for (SubInstruction &sub: sub_instructions) {
        //         processQueryFile << "Result for sub" << "(" << subid++ << "): \n";
        //         if (select == sub.param1) {
        //             for (const auto &[key, valueList]: sub.resultsMap) {
        //                 if (sub.relation == "Modifies" || sub.relation == "Uses") {
        //                     processQueryFile << key->get_node()->
        //                             to_string() <<
        //                             "";
        //                 } else {
        //                     processQueryFile << key->
        //                             get_command_no() <<
        //                             "";
        //                 }
        //
        //                 if (sub.relation == "Follows") {
        //                     processQueryFile << " follows ";
        //                 } else if (sub.relation == "Follows*") {
        //                     processQueryFile << " follows* ";
        //                 } else if (sub.relation == "Parent") {
        //                     processQueryFile << " is child of ";
        //                 } else if (sub.relation == "Parent*") {
        //                     processQueryFile << " is child* of ";
        //                 } else if (sub.relation == "Modifies") {
        //                     processQueryFile << " is modified by ";
        //                 } else if (sub.relation == "Uses") {
        //                     processQueryFile << " is used by ";
        //                 }
        //                 for (const std::shared_ptr<TNode> &node: valueList) {
        //                     processQueryFile << node->
        //                             get_command_no() <<
        //                             ", ";
        //                 }
        //                 processQueryFile << "\n";
        //             }
        //             processQueryFile << "\n";
        //         } else if (select == sub.param2) {
        //             for (const auto &[key, valueList]: sub.resultsMap) {
        //                 processQueryFile << key->
        //                         get_command_no() <<
        //                         "";
        //
        //                 if (sub.relation == "Follows") {
        //                     processQueryFile << " is followed by ";
        //                 } else if (sub.relation == "Follows*") {
        //                     processQueryFile << " is followed* by ";
        //                 } else if (sub.relation == "Parent") {
        //                     processQueryFile << " parents ";
        //                 } else if (sub.relation == "Parent*") {
        //                     processQueryFile << " parents* ";
        //                 } else if (sub.relation == "Modifies") {
        //                     processQueryFile << " modifies ";
        //                 } else if (sub.relation == "Uses") {
        //                     processQueryFile << " uses ";
        //                 }
        //
        //                 for (const std::shared_ptr<TNode> &node: valueList) {
        //                     if (sub.relation == "Modifies" || sub.relation == "Uses") {
        //                         processQueryFile << node->get_node()->
        //                                 to_string() <<
        //                                 "";
        //                     } else {
        //                         processQueryFile << node->
        //                                 get_command_no() <<
        //                                 "";
        //                     }
        //                     processQueryFile << ", ";
        //                 }
        //                 processQueryFile << "\n";
        //             }
        //             processQueryFile << "\n";
        //         } else {
        //             processQueryFile << "\n";
        //         }
        //     }
        // }

        void process_query(std::vector<std::shared_ptr<TNode> > procedureNodes,
                           std::vector<std::shared_ptr<TNode> > whileNodes,
                           std::vector<std::shared_ptr<TNode> > assignNodes,
                           std::vector<std::shared_ptr<TNode> > exprNodes,
                           std::vector<std::shared_ptr<TNode> > factorNodes) {
            std::cout << "Searching parsed code for: " << select << std::endl;

            bool first = true;

//            for (SubInstruction &sub: sub_instructions) {
//                if (first) {
//                    // Checking relation types and running functions
//                    if (sub.relation == "Follows") {
//                        std::cout << "Checking Follows relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Follows*") {
//                        std::cout << "Checking Follows* relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Parent") {
//                        std::cout << "Checking Parent relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Parent*") {
//                        std::cout << "Checking Parent* relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Modifies") {
//                        std::cout << "Checking Modifies relation\n";
//                        process_query_searching_nodes(assignNodes, factorNodes, sub);
//                    } else if (sub.relation == "Uses") {
//                        std::cout << "Checking Uses relation\n";
//                        process_query_searching_nodes(assignNodes, factorNodes, sub);
//                    } else {
//                        std::cout << "Unknown relation type\n";
//                    }
//                    first = false;
//                } else {
//                    if (sub.relation == "Follows") {
//                        std::cout << "Checking Follows relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Follows*") {
//                        std::cout << "Checking Follows* relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Parent") {
//                        std::cout << "Checking Parent relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Parent*") {
//                        std::cout << "Checking Parent* relation\n";
//                        process_query_searching_nodes(whileNodes, whileNodes, sub);
//                    } else if (sub.relation == "Modifies") {
//                        std::cout << "Checking Modifies relation\n";
//                        process_query_searching_nodes(exprNodes, factorNodes, sub);
//                    } else if (sub.relation == "Uses") {
//                        std::cout << "Checking Uses relation\n";
//                        process_query_searching_nodes(exprNodes, factorNodes, sub);
//                    } else {
//                        std::cout << "Unknown relation type\n";
//                    }
//                }
//
//            }
        }

        // void process_query_searching_nodes(std::vector<std::shared_ptr<TNode> > nodes1,
        //                                    std::vector<std::shared_ptr<TNode> > nodes2,
        //                                    SubInstruction &sub) {
        //     for (const auto &node1: nodes1) {
        //         for (const auto &node2: nodes2) {
        //             if (sub.relation == "Follows") {
        //                 if (!PKB::follows(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else if (sub.relation == "Follows*") {
        //                 if (!PKB::followsT(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else if (sub.relation == "Parent") {
        //                 if (!PKB::parent(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else if (sub.relation == "Parent*") {
        //                 if (!PKB::parentT(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else if (sub.relation == "Modifies") {
        //                 if (!PKB::modifies(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else if (sub.relation == "Uses") {
        //                 if (!PKB::uses(node1, node2)) continue;
        //                 process_query_searching(node1, node2, sub);
        //             } else {
        //                 std::cout << "Unknown relation type\n";
        //             }
        //         }
        //     }
        // }
        //
        // void process_query_searching(const std::shared_ptr<TNode> &node1,
        //                              const std::shared_ptr<TNode> &node2, SubInstruction &sub) {
        //     if (query_is_number(sub.param1)) {
        //         if (node1->get_command_no()
        //             == std::stoi(sub.param1)
        //         ) {
        //             query_check_with(node1, node2, sub);
        //         }
        //     } else if (query_is_number(sub.param2)) {
        //         if (node2->get_command_no() == std::stoi(sub.param2)
        //         ) {
        //             query_check_with(node1, node2, sub);
        //         }
        //     } else {
        //         query_check_with(node1, node2, sub);
        //     }
        // }
        //
        //
        // void query_check_with(const std::shared_ptr<TNode> &node1,
        //                       const std::shared_ptr<TNode> &node2, SubInstruction &sub) {
        //     if (select == sub.param1) {
        //         if (!sub.synonym.empty()) {
        //             if (sub.synonym == sub.param1) {
        //                 if (node1->get_command_no() == std::stoi(sub.value)) {
        //                     sub.resultsMap[node2].push_back(node1);
        //                 }
        //             } else if (sub.synonym == sub.param2) {
        //                 if (node2->get_command_no() == std::stoi(sub.value)) {
        //                     sub.resultsMap[node2].push_back(node1);
        //                 }
        //             }
        //         } else {
        //             sub.resultsMap[node2].push_back(node1);
        //         }
        //     } else if (select == sub.param2) {
        //         if (!sub.synonym.empty()) {
        //             if (sub.synonym == sub.param1) {
        //                 if (node1->get_command_no() == std::stoi(sub.value)) {
        //                     sub.resultsMap[node1].push_back(node2);
        //                 }
        //             } else if (sub.synonym == sub.param2) {
        //                 if (node2->get_command_no() == std::stoi(sub.value)) {
        //                     sub.resultsMap[node1].push_back(node2);
        //                 }
        //             }
        //         } else {
        //             sub.resultsMap[node1].push_back(node2);
        //         }
        //     }
        // }

        bool query_is_number(const std::string &str) {
            return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
        }
    };
} // query

#endif //INSTRUCTION_H
