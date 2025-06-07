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
    void processQueries() {
        // Relative file path
        std::filesystem::path basePath = std::filesystem::path(__FILE__).parent_path().parent_path();
        std::string inputFilePath = (basePath / "files" / "query_input.txt").string();
        std::string outputFilePath = (basePath / "files" / "query_output.txt").string();
        std::string partialOutputFilePath = (basePath / "files" / "query_output_partial.txt").string();
        // Check paths
        printf("%s\n", inputFilePath.c_str());
        printf("%s\n", outputFilePath.c_str());

        // Opening input file
        std::ifstream inputFile(inputFilePath);
        if (!inputFile.is_open()) {
            std::cerr << "Error when opening input file!" << std::endl;
            return;
        }

        // Opening output file
        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << "Error when opening output file!" << std::endl;
            return;
        }
        std::ofstream partialOutputFile(partialOutputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << "Error when opening output file!" << std::endl;
            return;
        }

        // Instructions vector
        std::vector<Instruction> instructions;
        // PKB node tree instance
//        PKB::instance().initialize();


        // Lists of nodes of certain types
        // std::vector<std::shared_ptr<TNode> > procedureNodes;
        // std::vector<std::shared_ptr<TNode> > whileNodes;
        // std::vector<std::shared_ptr<TNode> > assignNodes;
        // std::vector<std::shared_ptr<TNode> > exprNodes;
        // std::vector<std::shared_ptr<TNode> > factorNodes;

        // Checking node types and filling lists with nodes
        // for (const auto &node: allNodes) {
        //     if (node->get_tnode_type() == TN_PROCEDURE) {
        //         procedureNodes.push_back(node);
        //     }
        //     if (node->get_tnode_type() == TN_WHILE) {
        //         whileNodes.push_back(node);
        //     }
        //     if (node->get_tnode_type() == TN_ASSIGN) {
        //         assignNodes.push_back(node);
        //     } else if (node->get_tnode_type() == TN_EXPRESSION) {
        //         exprNodes.push_back(node);
        //     } else if (node->get_tnode_type() == TN_FACTOR) {
        //         factorNodes.push_back(node);
        //     }
        // }

        std::regex selectPattern(R"(^Select\s+(<[^>]+>|\w+)(.*)$)");
        std::regex relationPattern(
            R"((Follows\*?|Parent\*?|Modifies|Uses|Calls|Next\*?|Affects\*?)\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))");
        std::regex withPattern(
            R"(with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)\"|(\d+)|(\w+)))"
        );
        std::regex clauseSplitter(R"(\b(such\s+that|and|with)\b)");

        std::string line;
        while (std::getline(inputFile, line)) {
            BenchmarkTool tool(fmt::color::green_yellow, line);
            std::smatch m;
            if (!std::regex_match(line, m, selectPattern)) {
                std::cout << "No match for line: " << line << std::endl;
                continue;
            }
            tool.breakpoint(1);

            std::string selectPart = m[1].str();
            std::string rest = m[2].str();

            // Parse select variables (single or tuple)
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

            // Split rest by "such that" and "and"
            std::regex clauseSplit(R"(\b(such\s+that|and)\b)");
            std::sregex_token_iterator it(rest.begin(), rest.end(), clauseSplit, -1);
            std::sregex_token_iterator end;
            tool.breakpoint(2);

            for (; it != end; ++it) {
                std::string clause = trim(it->str());
                if (clause.empty()) continue;

                // Check if clause is a relation
                std::smatch relMatch;
                if (std::regex_search(clause, relMatch, relationPattern)) {
                    std::string relation = relMatch[1];
                    std::string left = trim(relMatch[2]);
                    std::string right = trim(relMatch[3]);

                    SubInstruction sub(relation, left, right);

                    // Check if there is a with clause inside this same clause (e.g. "Follows(a,b) with a.attr=5")
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

                // Check if clause is a pure "with" clause (without relation)
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

                    // Jeśli nie ma sub_instructions, tworzymy sztuczny sub (np. z relacją "With")
                    if (instr.sub_instructions.empty()) {
                        SubInstruction sub("With", "", "");
                        sub.add_synonym_constraint(syn);
                        instr.add_sub_instruction(sub);
                    } else {
                        // Dodajemy constraint do ostatniej sub_instruct
                        instr.get_last_sub_instruction().add_synonym_constraint(syn);
                    }
                }
            }
            tool.breakpoint(3);
            tool.reset();

            instructions.push_back(instr);
        }


        // std::cout << "\nProcedure Nodes: ";
        // for (const auto &node: procedureNodes) {
        //     std::cout << node->to_string() << ",";
        // }
        // std::cout << "\nWhile Nodes: ";
        // for (const auto &node: whileNodes) {
        //     std::cout << node->to_string() << ",";
        // }
        // std::cout << "\nAssign Nodes: ";
        // for (const auto &node: assignNodes) {
        //     //std::cout << node->to_string() << ",";
        //     std::cout << node->get_command_no() << ",";
        // }
        // std::cout << "\nExpression Nodes: ";
        // for (const auto &node: exprNodes) {
        //     //std::cout << node->to_string() << ",";
        //     std::cout << node->get_command_no() << ",";
        // }
        // std::cout << "\nFactor Nodes: ";
        // for (const auto &node: factorNodes) {
        //     //std::cout << node->to_string() << ",";
        //     std::cout << node->get_command_no() << ",";
        // }
        print_relations();
        for (auto &instr: instructions) {
            BenchmarkTool tool;
            instr.print_instruction();
            instr.process_query();
            instr.print_final_results_to_file(outputFile);
            instr.print_distinct_results_to_file(partialOutputFile);
            tool.reset();
        }

        print_relations();

        // Closing files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }

    void print_relations() {
        std::cout << "\nFollows" << std::endl;
        for (const auto &[left, right]: *PKB::followsRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nFollows*" << std::endl;
        for (const auto &[left, right]: *PKB::followsTRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nParent" << std::endl;
        for (const auto &[left, right]: *PKB::parentRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nParent*" << std::endl;
        for (const auto &[left, right]: *PKB::parentTRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nModifies" << std::endl;
        for (const auto &[left, right]: *PKB::modifiesRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.to_string() << "), ";
        }

        std::cout << "\nUses" << std::endl;
        for (const auto &[left, right]: *PKB::usesRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.to_string() << "), ";
        }

        std::cout << "\nCalls" << std::endl;
        for (const auto &[left, right]: *PKB::callsRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nNext" << std::endl;
        for (const auto &[left, right]: *PKB::nextRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << "\nNext*" << std::endl;
        for (const auto &[left, right]: *PKB::nextTRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() << "), ";
        }

        std::cout << std::endl;
    }


    std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\n\r");
        auto end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos)
            return "";
        return s.substr(start, end - start + 1);
    }

}
