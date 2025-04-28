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

        // Kolekcja instrukcji
        std::vector<Instruction> instructions;

        // Wzorzec dopasowania jednej pary relation(param1, param2) with optional with
        std::regex pattern(
            R"(Select\s+(\w+)\s+such\s+that\s+(.*))" // Łapiemy SELECT i całą resztę (relations)
        );

        std::smatch match;

        // Przetwarzanie linii
        std::string line;
        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, match, pattern)) {
                std::string selectSynonym = match[1].str();
                std::string instructionsPart = match[2].str();

                Instruction instr(selectSynonym);

                // Rozbij tekst na poszczególne relacje (oddzielone słowem 'and')
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
                        // Jeśli jest część "with"
                        sub.synonym = subMatch[5];
                        sub.attribute = subMatch[6];
                        if (subMatch[7].matched) sub.value = subMatch[7]; // "text"
                        else if (subMatch[8].matched) sub.value = subMatch[8]; // liczba
                        else if (subMatch[9].matched) sub.value = subMatch[9]; // zmienna
                    }

                    instr.addSubInstruction(sub);
                }

                // Dodaj gotową instrukcję
                instructions.push_back(instr);
            } else {
                std::cout << "No match: " << line << "\n";
            }
        }

        for (auto &instr: instructions) {
            instr.printInstruction();
            //instr.searchDatabase();
            //instr.printResults();
        }

        // Closing files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }

    void processRelation(std::ofstream &processRelationFile, const std::string &select,
                         const std::string &relation,
                         const std::string &param1, const std::string &param2,
                         const std::string &synonym, const std::string &attribute,
                         const std::string &value) {
        // Printing info
        std::cout << "Processing Relation:\n";
        std::cout << "  Select: " << select << "\n";
        std::cout << "  Relation: " << relation << "\n";
        std::cout << "  Param1: " << param1 << "\n";
        std::cout << "  Param2: " << param2 << "\n";

        // Additional info for "with"
        if (!synonym.empty()) {
            std::cout << "  With synonym: " << synonym << "\n";
            std::cout << "  Attribute: " << attribute << "\n";
            std::cout << "  Value: " << value << "\n";
        }

        // Using pkb class object
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
        // Adding query content to file
        processRelationFile << "Query result for: Select " << select << " such that " << relation << " (" << param1 <<
                ","
                << param2 << ")";
        if (!synonym.empty()) {
            processRelationFile << " with " << synonym << "." << attribute << "= " << value;
        }
        processRelationFile << "\n";

        // Checking relation types and running functions
        if (relation == "Follows") {
            std::cout << "Checking Follows relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: whileNodes) {
                    for (const auto &node2: whileNodes) {
                        if (!PKB::follows(node1, node2)) continue;

                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (!PKB::follows(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No follows relation\n";
            }
        } else if (relation == "Follows*") {
            std::cout << "Checking Follows* relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: whileNodes) {
                    for (const auto &node2: whileNodes) {
                        if (!PKB::followsT(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (!PKB::followsT(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No follows* relation\n";
            }
        } else if (relation == "Parent") {
            std::cout << "Checking Parent relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: whileNodes) {
                    for (const auto &node2: whileNodes) {
                        if (!PKB::parent(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (!PKB::parent(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No parent relation\n";
            }
        } else if (relation == "Parent*") {
            std::cout << "Checking Parent* relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: whileNodes) {
                    for (const auto &node2: whileNodes) {
                        if (!PKB::parentT(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (!PKB::parentT(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No parent* relation\n";
            }
        } else if (relation == "Modifies") {
            std::cout << "Checking Modifies relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: exprNodes) {
                    for (const auto &node2: factorNodes) {
                        if (!PKB::modifies(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: factorNodes) {
                    for (const auto &node1: exprNodes) {
                        if (!PKB::modifies(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No modifies relation\n";
            }
        } else if (relation == "Uses") {
            std::cout << "Checking Uses relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: exprNodes) {
                    for (const auto &node2: factorNodes) {
                        if (!PKB::uses(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: factorNodes) {
                    for (const auto &node1: exprNodes) {
                        if (!PKB::uses(node1, node2)) continue;
                        query_check_static_numbers(processRelationFile, node1, node2, select, relation, param1, param2,
                                                   synonym, value);
                    }
                }
            } else {
                std::cout << "No uses relation\n";
            }
        } else {
            std::cout << "Unknown relation type\n";
        }
    }

    void query_check_static_numbers(std::ofstream &queryFile, const std::shared_ptr<TNode> &node1,
                                    const std::shared_ptr<TNode> &node2, const std::string &select,
                                    const std::string &relation,
                                    const std::string &param1,
                                    const std::string &param2,
                                    const std::string &synonym, const std::string &value) {
        if (query_is_number(param1)) {
            if (node1->get_command_no()
                == std::stoi(param1)
            ) {
                query_check_with(queryFile, node1, node2, select, relation, param1,
                                 param2,
                                 synonym,
                                 value);
            }
        } else if (query_is_number(param2)) {
            if (node2->get_command_no() == std::stoi(param2)
            ) {
                query_check_with(queryFile, node1, node2, select, relation, param1,
                                 param2,
                                 synonym,
                                 value);
            }
        } else {
            query_check_with(queryFile, node1, node2, select, relation, param1, param2,
                             synonym,
                             value);
        }
    }


    void query_check_with(std::ofstream &queryFile, const std::shared_ptr<TNode> &node1,
                          const std::shared_ptr<TNode> &node2, const std::string &select, const std::string &relation,
                          const std::string &param1,
                          const std::string &param2,
                          const std::string &synonym, const std::string &value) {
        if (select == param1) {
            if (!synonym.empty()) {
                if (synonym == param1) {
                    if (node1->get_command_no() == std::stoi(value)) {
                        to_file_param1(queryFile, node1, node2, relation);
                    }
                } else if (synonym == param2) {
                    if (node2->get_command_no() == std::stoi(value)) {
                        to_file_param1(queryFile, node1, node2, relation);
                    }
                }
            } else {
                to_file_param1(queryFile, node1, node2, relation);
            }
        } else if (select == param2) {
            if (!synonym.empty()) {
                if (synonym == param1) {
                    if (node1->get_command_no() == std::stoi(value)) {
                        to_file_param2(queryFile, node1, node2, relation);
                    }
                } else if (synonym == param2) {
                    if (node2->get_command_no() == std::stoi(value)) {
                        to_file_param2(queryFile, node1, node2, relation);
                    }
                }
            } else {
                to_file_param2(queryFile, node1, node2, relation);
            }
        }
    }

    bool query_is_number(const std::string &str) {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    void to_file_param1(std::ofstream &to_file_param1, const std::shared_ptr<TNode> &node1,
                        const std::shared_ptr<TNode> &node2, const std::string &relation) {
        to_file_param1 << node1->get_command_no();

        if (relation == "Follows") {
            to_file_param1 << " follows before ";
        } else if (relation == "Follows*") {
            to_file_param1 << " follows* before ";
        } else if (relation == "Parent") {
            to_file_param1 << " parents ";
        } else if (relation == "Parent*") {
            to_file_param1 << " parents* ";
        } else if (relation == "Modifies") {
            to_file_param1 << " modifies ";
        } else if (relation == "Uses") {
            to_file_param1 << " uses ";
        }

        if (relation == "Modifies" || relation == "Uses") {
            to_file_param1 << node2->
                    get_node()->to_string() <<
                    "\n";
        } else {
            to_file_param1 << node2->
                    get_command_no() <<
                    "\n";
        }
    }

    void to_file_param2(std::ofstream &to_file_param2, const std::shared_ptr<TNode> &node1,
                        const std::shared_ptr<TNode> &node2, const std::string &relation) {
        if (relation == "Modifies" || relation == "Uses") {
            to_file_param2 << node2->get_node()->to_string();
        } else {
            to_file_param2 << node2->
                    get_command_no();
        }

        if (relation == "Follows") {
            to_file_param2 << " follows ";
        } else if (relation == "Follows*") {
            to_file_param2 << " follows* ";
        } else if (relation == "Parent") {
            to_file_param2 << " is child of ";
        } else if (relation == "Parent*") {
            to_file_param2 << " is child* of ";
        } else if (relation == "Modifies") {
            to_file_param2 << " is modified by ";
        } else if (relation == "Uses") {
            to_file_param2 << " is used by ";
        }

        to_file_param2 << node1->get_command_no();
        to_file_param2 << "\n";
    }
}
