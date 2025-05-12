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
        std::vector<std::shared_ptr<TNode> > procedureNodes;
        std::vector<std::shared_ptr<TNode> > whileNodes;
        std::vector<std::shared_ptr<TNode> > assignNodes;
        std::vector<std::shared_ptr<TNode> > exprNodes;
        std::vector<std::shared_ptr<TNode> > factorNodes;

        // Checking node types and filling lists with nodes
        for (const auto &node: allNodes) {
            if (node->get_tnode_type() == TN_PROCEDURE) {
                procedureNodes.push_back(node);
            }
            if (node->get_tnode_type() == TN_WHILE) {
                whileNodes.push_back(node);
            }
            if (node->get_tnode_type() == TN_ASSIGN) {
                assignNodes.push_back(node);
            } else if (node->get_tnode_type() == TN_EXPRESSION) {
                exprNodes.push_back(node);
            } else if (node->get_tnode_type() == TN_FACTOR) {
                factorNodes.push_back(node);
            }
        }

        // Select pattern with such that
        std::regex pattern(
            R"(Select\s+(\w+)\s+such\s+that\s+(.*))"
        );

        std::smatch match;

        // Line processing
        std::string line;
        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, match, pattern)) {
                std::string selectSynonym = match[1].str();
                std::string instructionsPart = match[2].str();

                Instruction instr(selectSynonym);

                // Dividing into "and" parts
                std::regex relationPattern(
                    R"((Follows\*?|Parent\*?|Modifies|Uses)\s*\(\s*([^,]+)\s*,\s*([^)]+)\)\s*(with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)|(\d+)|(\w+)))?)"
                );
                auto begin = std::sregex_iterator(instructionsPart.begin(), instructionsPart.end(), relationPattern);
                auto end = std::sregex_iterator();

                for (auto it = begin; it != end; ++it) {
                    std::smatch subMatch = *it;

                    SubInstruction sub;

                    sub.relation = subMatch[1];
                    sub.param1 = subMatch[2];
                    sub.param2 = subMatch[3];

                    if (subMatch[5].matched) {
                        // If there is a part with "with"
                        sub.synonym = subMatch[5];
                        sub.attribute = subMatch[6];
                        if (subMatch[7].matched) sub.value = subMatch[7]; // "text"
                        else if (subMatch[8].matched) sub.value = subMatch[8]; // value
                        else if (subMatch[9].matched) sub.value = subMatch[9]; // variable
                    }

                    instr.add_sub_instruction(sub);
                }

                // Add finished instruction entity
                instructions.push_back(instr);
            } else {
                std::cout << "No match: " << line << "\n";
            }
        }

        for (auto &instr: instructions) {
            instr.print_instruction();
            instr.process_query(procedureNodes, whileNodes, assignNodes, exprNodes, factorNodes);
            instr.print_subquery_results_to_file(outputFile);
        }

        // Closing files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }
}
