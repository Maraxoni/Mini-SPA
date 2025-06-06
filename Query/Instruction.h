//
// Created by Marax on 27.04.2025.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <iostream>
#include <fstream>
#include <list>
#include <memory>
#include <set>
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
        std::vector<std::string> select_variables;
        std::vector<SubInstruction> sub_instructions;

        std::vector<std::unordered_map<std::string, int> > variableResults;

        Instruction(const std::vector<std::string> &selects) : select_variables(selects) {
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
            std::cout << "Select ";
            for (size_t i = 0; i < select_variables.size(); ++i) {
                std::cout << select_variables[i];
                if (i != select_variables.size() - 1) std::cout << ", ";
            }

            if (!sub_instructions.empty()) {
                std::cout << " such that ";
            }

            for (size_t i = 0; i < sub_instructions.size(); ++i) {
                const auto &sub = sub_instructions[i];
                if (i != 0) std::cout << " and ";
                std::cout << sub.relation << "(" << sub.left_param << ", " << sub.right_param << ")";

                for (size_t j = 0; j < sub.synonym_constraints.size(); ++j) {
                    const auto &syn = sub.synonym_constraints[j];
                    std::cout << (j == 0 ? " with " : " and ")
                            << syn.synonym << "." << syn.attribute << " = " << syn.value;
                }
            }

            std::cout << std::endl;
        }

        void print_instruction_to_file(std::ofstream &file) const {
            // 1. Zbieramy wszystkie zmienne, które pojawiły się w wynikach (variableResults)
            std::set<std::string> result_variables;
            for (const auto &row : variableResults) {
                for (const auto &[key, _] : row) {
                    result_variables.insert(key);
                }
            }

            // 2. Wypisujemy deklarację stmt a, b, ...;
            file << "stmt ";
            size_t idx = 0;
            for (const auto &var : result_variables) {
                file << var;
                if (idx != result_variables.size() - 1) file << ", ";
                ++idx;
            }
            file << ";\n";

            file << "Select ";
            for (size_t i = 0; i < select_variables.size(); ++i) {
                file << select_variables[i];
                if (i != select_variables.size() - 1) file << ", ";
            }

            if (!sub_instructions.empty()) {
                file << " such that ";
            }

            for (size_t i = 0; i < sub_instructions.size(); ++i) {
                const auto &sub = sub_instructions[i];
                if (i != 0) file << " and ";
                file << sub.relation << "(" << sub.left_param << ", " << sub.right_param << ")";
                for (size_t j = 0; j < sub.synonym_constraints.size(); ++j) {
                    const auto &syn = sub.synonym_constraints[j];
                    file << (j == 0 ? " with " : " and ")
                            << syn.synonym << "." << syn.attribute << " = " << syn.value;
                }
            }

            file << "\n";
        }

        void print_final_results_to_file(std::ofstream &file) const {
            print_instruction_to_file(file);

            std::set<std::vector<int> > seen_rows;

            for (const auto &row: variableResults) {
                std::vector<int> result_line;
                bool all_present = true;

                for (const auto &sel: select_variables) {
                    if (row.count(sel)) {
                        result_line.push_back(row.at(sel));
                    } else {
                        all_present = false;
                        break;
                    }
                }

                if (all_present) {
                    seen_rows.insert(result_line);
                }
            }

            size_t row_idx = 0;
            for (const auto &line : seen_rows) {
                for (size_t i = 0; i < line.size(); ++i) {
                    file << line[i];
                    if (i != line.size() - 1) {
                        file << ", ";
                    }
                }
                if (row_idx != seen_rows.size() - 1) {
                    file << ", ";
                }
                ++row_idx;
            }
            file << "\n";
        }

        void print_distinct_results_to_file(std::ofstream &file) const {
            if (select_variables.empty()) {
                file << "No select variable specified.\n";
                return;
            }

            // Wypisz zapytanie
            print_instruction_to_file(file);

            if (select_variables.size() == 1) {
                // Grupa wyników dla jednej zmiennej SELECT
                const std::string &select = select_variables[0];
                file << "Grouped results for Select " << select << ":\n";

                std::unordered_map<int, std::unordered_set<std::string> > resultGroups;

                for (const auto &row: variableResults) {
                    if (!row.count(select)) continue;
                    int mainVal = row.at(select);
                    // Zbierz wszystkie pozostałe wartości jako stringi (z kluczem=wartością)
                    for (const auto &[key, val]: row) {
                        if (key == select) continue;
                        resultGroups[mainVal].insert(std::to_string(val));
                    }
                }

                for (const auto &[mainVal, relatedVals]: resultGroups) {
                    file << mainVal << ": ";
                    bool first = true;
                    for (const auto &val: relatedVals) {
                        if (!first) file << ", ";
                        file << val;
                        first = false;
                    }
                    file << " | ";
                }
                file << "\n";
            } else {
                // Grupa wyników dla wielu zmiennych SELECT
                // Mapujemy wartość pierwszej zmiennej na zestawy pozostałych zmiennych

                file << "Grouped results for Select ";
                for (size_t i = 0; i < select_variables.size(); ++i) {
                    file << select_variables[i];
                    if (i != select_variables.size() - 1) file << ", ";
                }
                file << ":\n";

                // map: pierwsza zmienna -> set of tuples (pozostałe zmienne)
                std::unordered_map<int, std::set<std::vector<int> > > groupedResults;

                for (const auto &row: variableResults) {
                    if (!row.count(select_variables[0])) continue;
                    int keyVal = row.at(select_variables[0]);

                    std::vector<int> others;
                    bool all_present = true;
                    for (size_t i = 1; i < select_variables.size(); ++i) {
                        if (!row.count(select_variables[i])) {
                            all_present = false;
                            break;
                        }
                        others.push_back(row.at(select_variables[i]));
                    }
                    if (all_present) {
                        groupedResults[keyVal].insert(others);
                    }
                }

                for (const auto &[keyVal, tuples]: groupedResults) {
                    file << keyVal << ": ";
                    bool firstTuple = true;
                    for (const auto &tuple: tuples) {
                        if (!firstTuple) file << "; ";
                        file << "(";
                        for (size_t i = 0; i < tuple.size(); ++i) {
                            file << tuple[i];
                            if (i != tuple.size() - 1) file << ", ";
                        }
                        file << ")";
                        firstTuple = false;
                    }
                    file << " | ";
                }
                file << "\n";
            }
        }

        void process_query() {
            if (select_variables.empty()) {
                std::cout << "No select variables specified." << std::endl;
                return;
            }

            const std::string &first_select = select_variables[0];
            std::cout << "Searching parsed code for: " << first_select << std::endl;

            std::vector<std::unordered_map<std::string, int> > intermediateResults;

            for (const SubInstruction &sub: sub_instructions) {
                std::shared_ptr<std::vector<std::pair<TNode, TNode> > > relationVec;

                // Mapowanie nazw relacji do wektorów z PKB
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
                } else if (sub.relation == "Calls") {
                    relationVec = PKB::callsRelations;
                } else if (sub.relation == "Next") {
                    relationVec = PKB::nextRelations;
                } else if (sub.relation == "Next*") {
                    relationVec = PKB::nextTRelations;
                } else {
                    std::cout << "Unknown relation: " << sub.relation << "\n";
                    continue;
                }

                std::vector<std::unordered_map<std::string, int> > currentRelationResults;

                // Przechodzimy przez wszystkie pary w relacji
                for (const auto &[left, right]: *relationVec) {
                    // Sprawdzamy zgodność parametrów z wartościami w relacji
                    // Jeśli parametr jest liczbą, to porównujemy bezpośrednio
                    // Jeśli jest zmienną, to dodajemy do wyniku
                    bool leftMatches = query_is_number(sub.left_param)
                                           ? (std::stoi(sub.left_param) == left.get_command_no())
                                           : true;

                    bool rightMatches = query_is_number(sub.right_param)
                                            ? (std::stoi(sub.right_param) == right.get_command_no())
                                            : true;

                    if (!leftMatches || !rightMatches) continue;

                    std::unordered_map<std::string, int> row;
                    if (!query_is_number(sub.left_param)) {
                        row[sub.left_param] = left.get_command_no();
                    }
                    if (!query_is_number(sub.right_param)) {
                        row[sub.right_param] = right.get_command_no();
                    }

                    currentRelationResults.push_back(std::move(row));
                }

                // Scalanie wyników (join)
                if (intermediateResults.empty()) {
                    intermediateResults = std::move(currentRelationResults);
                } else {
                    intermediateResults = join_results(intermediateResults, currentRelationResults);
                }
            }

            variableResults = std::move(intermediateResults);
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
