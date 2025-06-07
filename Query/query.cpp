#include "query.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

#include "Instruction.h"
#include "../pkb.h"

namespace query {
    void print_relations() {
        std::cout << "\nFollows" << std::endl;
        for (const auto &[left, right]: *PKB::followsRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nFollows*" << std::endl;
        for (const auto &[left, right]: *PKB::followsTRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nParent" << std::endl;
        for (const auto &[left, right]: *PKB::parentRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nParent*" << std::endl;
        for (const auto &[left, right]: *PKB::parentTRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nModifies" << std::endl;
        for (const auto &[left, right]: *PKB::modifiesRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.to_string() << "), ";
        }

        std::cout << "\nUses" << std::endl;
        for (const auto &[left, right]: *PKB::usesRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.to_string() << "), ";
        }

        std::cout << "\nCalls" << std::endl;
        for (const auto &[left, right]: *PKB::callsRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nCallsT" << std::endl;
        for (const auto &[left, right]: *PKB::callsTRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nNext" << std::endl;
        for (const auto &[left, right]: *PKB::nextRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << "\nNext*" << std::endl;
        for (const auto &[left, right]: *PKB::nextTRelations) {
            std::cout << "(" << left.get_node()->mLineNumber << ": " << right.get_node()->mLineNumber << "), ";
        }

        std::cout << std::endl;
    }


    std::string trim(const std::string &s) {
        auto start = s.find_first_not_of(" \t\n\r");
        auto end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos)
            return "";
        return s.substr(start, end - start + 1);
    }

    void processQueries() {
        print_relations();
    }


    std::string processPQL(const std::string &declarations, const std::string &selectLine, bool instruction_cond) {
        try {
            std::string fullQuery = declarations + "\n" + selectLine;

            std::regex selectPattern(R"(^Select\s+(<[^>]+>|\w+)(.*)$)");
            std::regex relationPattern(
                R"((Follows\*?|Parent\*?|Modifies|Uses|Calls|Next\*?|Affects\*?)\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))");
            std::regex withPattern(
                R"(with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)\"|(\d+)|(\w+)))"
            );
            std::regex clauseSplitter(R"(\b(such\s+that|and|with)\b)");


            std::smatch m;
            if (!std::regex_match(selectLine, m, selectPattern)) {
                return "# Syntax error in query";
            }

            std::string selectPart = m[1].str();
            std::string rest = m[2].str();

            std::vector<std::string> selectVars;
            if (selectPart.front() == '<') {
                std::string inside = selectPart.substr(1, selectPart.size() - 2);
                std::stringstream ss(inside);
                std::string var;
                while (std::getline(ss, var, ',')) {
                    selectVars.push_back(trim(var));
                }
            } else {
                selectVars.push_back(trim(selectPart));
            }

            Instruction instr(selectVars);

            std::regex clauseSplit(R"(\b(such\s+that|and|with)\b)");
            std::sregex_token_iterator it(rest.begin(), rest.end(), clauseSplit, -1);
            std::sregex_token_iterator end;

            for (; it != end; ++it) {
                std::string clause = trim(it->str());
                if (clause.empty()) continue;

                std::smatch relMatch;
                if (std::regex_search(clause, relMatch, relationPattern)) {
                    std::string relation = relMatch[1];
                    std::string left = trim(relMatch[2]);
                    std::string right = trim(relMatch[3]);

                    SubInstruction sub(relation, left, right);

                    // Dodajemy wszystkie constrainty from this clause
                    auto witBegin = std::sregex_iterator(clause.begin(), clause.end(), withPattern);
                    auto witEnd = std::sregex_iterator();
                    for (auto wit = witBegin; wit != witEnd; ++wit) {
                        std::smatch wMatch = *wit;
                        SynonymConstraint syn;
                        syn.synonym = wMatch[1];
                        syn.attribute = wMatch[2];
                        if (wMatch[3].matched) syn.value = wMatch[3];
                        else if (wMatch[4].matched) syn.value = wMatch[4];
                        else if (wMatch[5].matched) syn.value = wMatch[5];
                        sub.add_synonym_constraint(syn);
                    }

                    instr.add_sub_instruction(sub);
                    continue;
                }

                // Pure "with" clause (without relation)
                auto witBegin = std::sregex_iterator(clause.begin(), clause.end(), withPattern);
                auto witEnd = std::sregex_iterator();
                for (auto wit = witBegin; wit != witEnd; ++wit) {
                    std::smatch wMatch = *wit;
                    SynonymConstraint syn;
                    syn.synonym = wMatch[1];
                    syn.attribute = wMatch[2];
                    if (wMatch[3].matched) syn.value = wMatch[3];
                    else if (wMatch[4].matched) syn.value = wMatch[4];
                    else if (wMatch[5].matched) syn.value = wMatch[5];

                    if (instr.sub_instructions.empty()) {
                        SubInstruction sub("With", "", "");
                        sub.add_synonym_constraint(syn);
                        instr.add_sub_instruction(sub);
                    } else {
                        instr.get_last_sub_instruction().add_synonym_constraint(syn);
                    }
                }
            }

            instr.process_query();
            if (instruction_cond == true) {
                std::string result = instr.get_variables_with_types_string() + instr.get_instruction_string() + instr.get_result_string();
                return result;
            } else {
                return instr.get_result_string();
            }

        } catch (const std::exception &e) {
            return std::string("# Exception: ") + e.what();
        } catch (...) {
            return "# Unknown exception during query processing";
        }
    }
}
