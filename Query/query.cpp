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

        // Instructions vector
        std::vector<Instruction> instructions;
        // PKB node tree instance
        std::shared_ptr<TNode> rootNode = PKB::instance().get_ast();
        std::vector<std::shared_ptr<TNode> > allNodes = PKB::get_ast_as_list(rootNode);

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
        // Select pattern with such that or and
        std::regex selectPattern(
            R"(Select\s+(\w+)\s+(such\s+that|and)\s+(.*))"
        );

        // Relations: Follows, Parent, Modifies, Uses
        std::regex relationPattern(
            R"((Follows\*?|Parent\*?|Modifies|Uses)\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))"
        );

        // "with" clauses
        std::regex withPattern(
            R"(with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)|(\d+)|(\w+)))"
        );

        std::smatch match;
        std::string line;
        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, match, selectPattern)) {
                std::string selectSynonym = match[1].str();
                std::string instructionsPart = match[3].str();

                Instruction instr(selectSynonym);

                // Rozbijamy na części po "such that" i "and" (bez usuwania delimitera)
                std::regex clauseSplitter(R"(\s+(such\s+that|and)\s+)");
                std::sregex_token_iterator it(instructionsPart.begin(), instructionsPart.end(), clauseSplitter, -1);
                std::sregex_token_iterator end;

                SubInstruction *lastSub = nullptr;

                for (; it != end; ++it) {
                    std::string part = it->str();

                    // Searching relations
                    std::smatch relMatch;
                    if (std::regex_search(part, relMatch, relationPattern)) {
                        SubInstruction sub(relMatch[1], relMatch[2], relMatch[3]);
                        instr.add_sub_instruction(sub);
                        lastSub = &instr.get_last_sub_instruction();
                    }

                    // Searching clauses
                    auto withBegin = std::sregex_iterator(part.begin(), part.end(), withPattern);
                    auto withEnd = std::sregex_iterator();
                    for (auto wit = withBegin; wit != withEnd; ++wit) {
                        if (!lastSub) continue; // Jeśli nie ma relacji, ignoruj "with"
                        std::smatch wMatch = *wit;
                        SynonymConstraint syn;
                        syn.synonym = wMatch[1];
                        syn.attribute = wMatch[2];

                        if (wMatch[3].matched) syn.value = wMatch[3]; // "quoted string"
                        else if (wMatch[4].matched) syn.value = wMatch[4]; // number
                        else if (wMatch[5].matched) syn.value = wMatch[5]; // variable

                        lastSub->add_synonym_constraint(syn);
                    }
                }

                instructions.push_back(instr);
            } else {
                std::cout << "No match: " << line << "\n";
            }
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
            instr.print_instruction();
            instr.process_query();
            //instr.print_final_results_to_file(outputFile);
            instr.print_distinct_results_to_file(outputFile);
        }

        // Closing files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }

    void print_relations() {
        std::cout << "\nFollows" << std::endl;
        for (const auto& [left, right] : *PKB::followsRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() <<"), ";
        }
        std::cout << "\nFollows*" << std::endl;
        for (const auto& [left, right] : *PKB::followsTRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() <<"), ";
        }
        std::cout << "\nParent" << std::endl;
        for (const auto& [left, right] : *PKB::parentRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() <<"), ";
        }
        std::cout << "\nParent*" << std::endl;
        for (const auto& [left, right] : *PKB::parentTRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.get_command_no() <<"), ";
        }
        std::cout << "\nModifies" << std::endl;
        for (const auto& [left, right] : *PKB::modifiesRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.to_string() <<"), ";
        }
        std::cout << "\nUses" << std::endl;
        for (const auto& [left, right] : *PKB::usesRelations) {
            std::cout << "(" << left.get_command_no() << ": " << right.to_string() <<"), ";
        }
    }
}
