#include "query.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Instruction.h"
#include "../pkb.h"

namespace query {
    // Debug flag to control debug output
    bool DEBUG = false;

    // Helper function for debug output
    void debug_log(const std::string &msg) {
        if (DEBUG) {
            std::cout << "[DEBUG] " << msg << std::endl;
        }
    }

    // Print all relations stored in PKB for debugging
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

    // Trim whitespace from both ends of the string
    std::string trim(const std::string &s) {
        auto start = s.find_first_not_of(" \t\n\r");
        auto end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos)
            return "";
        return s.substr(start, end - start + 1);
    }

    // Split string by delimiter and trim each part
    std::vector<std::string> split_and_trim(const std::string &s, char delimiter) {
        std::vector<std::string> res;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            std::string t = trim(item);
            if (!t.empty())
                res.push_back(t);
        }
        return res;
    }

    // Process declarations like: "stmt a, b;"
    void process_declarations(const std::string &declarations, Instruction &instr) {
        std::regex declPattern(R"((stmt|assign|while|if|call|variable|procedure|constant)\s+([^;]+);)");
        auto declBegin = std::sregex_iterator(declarations.begin(), declarations.end(), declPattern);
        auto declEnd = std::sregex_iterator();

        for (auto it = declBegin; it != declEnd; ++it) {
            std::string type = (*it)[1].str();
            std::string varList = (*it)[2].str();
            std::vector<std::string> vars = split_and_trim(varList, ',');
            for (auto &var: vars) {
                instr.variable_types[var] = type;
            }
        }
    }

    // Main query processor
    std::string processPQL(const std::string &declarations, const std::string &selectLine, bool instruction_cond) {
        try {
            std::regex selectPattern(R"(^Select\s+(<[^>]+>|\w+)(.*)$)");
            std::smatch m;

            if (!std::regex_match(selectLine, m, selectPattern)) {
                return "# Syntax error in query";
            }

            std::string selectPart = m[1].str();
            std::string rest = m[2].str();

            debug_log("Select part: \"" + selectPart + "\"");
            debug_log("Rest of query after Select: \"" + rest + "\"");

            // Parse select variables
            std::vector<std::string> selectVars;
            if (selectPart.front() == '<') {
                std::string inside = selectPart.substr(1, selectPart.size() - 2);
                selectVars = split_and_trim(inside, ',');
            } else {
                selectVars.push_back(trim(selectPart));
            }

            Instruction instr(selectVars);

            // Process declarations
            process_declarations(declarations, instr);

            // Regex patterns for clauses:
            std::regex relationPattern(
                R"((Follows\*?|Parent\*?|Modifies|Uses|Calls|CallsT|Next\*?|Affects\*?)\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))");
            std::regex patternPattern(R"(pattern\s+(\w+)\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))");
            std::regex withPattern(
                R"xxx(with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)"|(\d+)|(\w+\.\w+)|(\w+)))xxx"
            );
            std::regex clauseSplitter(R"xxx(\b(such\s+that|pattern|and)\b)xxx", std::regex::icase);

            std::sregex_token_iterator it(rest.begin(), rest.end(), clauseSplitter, -1);
            std::sregex_token_iterator end;

            SubInstruction *lastSubInstr = nullptr;

            for (; it != end; ++it) {
                std::string clause = trim(it->str());
                if (clause.empty()) continue;

                debug_log("Clause parsed: \"" + clause + "\"");

                // Look for relation clause
                std::smatch relMatch;
                if (std::regex_search(clause, relMatch, relationPattern)) {
                    std::string relation = trim(relMatch[1]);
                    std::string left = trim(relMatch[2]);
                    std::string right = trim(relMatch[3]);

                    debug_log("Relation found: " + relation + "(" + left + ", " + right + ")");

                    SubInstruction sub(relation, left, right);

                    // Add all with clauses related to this clause
                    auto witBegin = std::sregex_iterator(clause.begin(), clause.end(), withPattern);
                    auto witEnd = std::sregex_iterator();

                    for (auto wit = witBegin; wit != witEnd; ++wit) {
                        SynonymConstraint syn;
                        syn.synonym = (*wit)[1];
                        syn.attribute = (*wit)[2];
                        if ((*wit)[3].matched) syn.value = (*wit)[3]; // string literal
                        else if ((*wit)[4].matched) syn.value = (*wit)[4]; // number literal
                        else if ((*wit)[5].matched) syn.value = (*wit)[5]; // synonym.attribute
                        else if ((*wit)[6].matched) syn.value = (*wit)[6]; // synonym

                        debug_log("  With clause: " + syn.synonym + "." + syn.attribute + " = " + syn.value);

                        sub.add_synonym_constraint(syn);
                    }

                    instr.add_sub_instruction(sub);
                    lastSubInstr = &instr.sub_instructions.back();
                    continue;
                }

                // Look for pattern clause
                std::smatch patMatch;
                if (std::regex_search(clause, patMatch, patternPattern)) {
                    std::string synonym = trim(patMatch[1]);
                    std::string left = trim(patMatch[2]);
                    std::string right = trim(patMatch[3]);

                    debug_log("Pattern found: pattern " + synonym + "(" + left + ", " + right + ")");

                    SubInstruction sub("Pattern", left, right);
                    sub.set_pattern_synonym(synonym);

                    auto witBegin = std::sregex_iterator(clause.begin(), clause.end(), withPattern);
                    auto witEnd = std::sregex_iterator();

                    for (auto wit = witBegin; wit != witEnd; ++wit) {
                        SynonymConstraint syn;
                        syn.synonym = (*wit)[1];
                        syn.attribute = (*wit)[2];
                        if ((*wit)[3].matched) syn.value = (*wit)[3];
                        else if ((*wit)[4].matched) syn.value = (*wit)[4];
                        else if ((*wit)[5].matched) syn.value = (*wit)[5];
                        else if ((*wit)[6].matched) syn.value = (*wit)[6];

                        debug_log("  With clause: " + syn.synonym + "." + syn.attribute + " = " + syn.value);

                        sub.add_synonym_constraint(syn);
                    }

                    instr.add_sub_instruction(sub);
                    lastSubInstr = &instr.sub_instructions.back();
                    continue;
                }

                // 'and' clause or additional with clauses, add to last subinstruction
                if (lastSubInstr != nullptr) {
                    auto witBegin = std::sregex_iterator(clause.begin(), clause.end(), withPattern);
                    auto witEnd = std::sregex_iterator();

                    bool added = false;
                    for (auto wit = witBegin; wit != witEnd; ++wit) {
                        SynonymConstraint syn;
                        syn.synonym = (*wit)[1];
                        syn.attribute = (*wit)[2];
                        if ((*wit)[3].matched) syn.value = (*wit)[3];
                        else if ((*wit)[4].matched) syn.value = (*wit)[4];
                        else if ((*wit)[5].matched) syn.value = (*wit)[5];
                        else if ((*wit)[6].matched) syn.value = (*wit)[6];

                        debug_log(
                            "Adding with clause to last SubInstruction: " + syn.synonym + "." + syn.attribute + " = " +
                            syn.value);

                        lastSubInstr->add_synonym_constraint(syn);
                        added = true;
                    }
                    if (added) continue;

                    // Can be extended for other relations after 'and'
                }
            }

            instr.process_query();

            if (instruction_cond) {
                return instr.get_variables_with_types_string() + instr.get_instruction_string() + instr.
                       get_result_string();
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
