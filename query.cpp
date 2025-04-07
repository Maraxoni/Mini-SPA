#include "query.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

#include "parser.h"
#include "pkb.h"

namespace query {
    void processQueries() {
        // Relative file path
        std::filesystem::path basePath = std::filesystem::path(__FILE__).parent_path();
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

        // Loaded line
        std::string line;
        // Searching pattern
        std::regex pattern(
            R"(Select\s+(\w+)\s+such\s+that\s+(Follows\*?|Parent\*?|Modifies|Uses)\s*\(\s*((?:"[^"]+"|\w+|\d+))\s*,\s*((?:"[^"]+"|\w+|\d+))\s*\)(?:\s+with\s+(\w+)\.([\w#]+)\s*=\s*(?:"([^"]+)\"|(\d+)|(\w+)))?)"
        );

        std::smatch match;

        while (std::getline(inputFile, line)) {
            if (std::regex_match(line, match, pattern)) {
                std::cout << "Matched line: " << line << "\n";

                // Match values
                std::string select = match[1];
                std::string relation = match[2];
                std::string param1 = match[3];
                std::string param2 = match[4];

                // Variables for "with" if exists
                std::string synonym = match[5].matched ? match[5].str() : "";
                std::string attribute = match[6].matched ? match[6].str() : "";
                std::string value = "";

                // Match values if exists
                if (match[7].matched) {
                    value = match[7]; // text
                } else if (match[8].matched) {
                    value = match[8]; // int
                } else if (match[9].matched) {
                    value = match[9]; // id
                }

                // Processing relation
                processRelation(outputFile, select, relation, param1, param2, synonym, attribute, value);
            } else {
                std::cout << "No match: " << line << "\n";
            }
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
        // Loading code file
        std::string path = "../files/input_min2.txt";
        auto parser = std::make_shared<Parser>();
        if (!parser->initialize_by_file(path)) {
            return;
        }
        // Using pkb class object
        PKB pkb = PKB(parser);
        std::shared_ptr<TNode> rootNode = pkb.build_AST();
        std::vector<std::shared_ptr<TNode> > allNodes = pkb.get_ast_as_list(rootNode);
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
                        if (pkb.follows(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (pkb.follows(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
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
                        if (pkb.followsT(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: whileNodes) {
                    for (const auto &node1: whileNodes) {
                        if (pkb.followsT(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else {
                std::cout << "No follows* relation\n";
            }
        } else if (relation == "Parent") {
            std::cout << "Checking Parent relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: exprNodes) {
                    for (const auto &node2: exprNodes) {
                        if (pkb.parent(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: exprNodes) {
                    for (const auto &node1: exprNodes) {
                        if (pkb.parent(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else {
                std::cout << "No parent relation\n";
            }
        } else if (relation == "Parent*") {
            std::cout << "Checking Parent* relation\n";
            if (select == param1) {
                // For select == param1: Finding node 2 that comes after node 1
                for (const auto &node1: exprNodes) {
                    for (const auto &node2: exprNodes) {
                        if (pkb.parentT(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: exprNodes) {
                    for (const auto &node1: exprNodes) {
                        if (pkb.parentT(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
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
                        if (pkb.modifies(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: factorNodes) {
                    for (const auto &node1: exprNodes) {
                        if (pkb.modifies(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
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
                    for (const auto &node2: exprNodes) {
                        if (pkb.uses(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else if (select == param2) {
                // For select == param2: Finding node 1 that comes before node 2
                for (const auto &node2: exprNodes) {
                    for (const auto &node1: exprNodes) {
                        if (pkb.uses(node1, node2)) {
                            toFileQueryResult(processRelationFile, node1, node2, select, relation, param1, param2,
                                              synonym,
                                              value);
                        }
                    }
                }
            } else {
                std::cout << "No uses relation\n";
            }
        } else {
            std::cout << "Unknown relation type\n";
        }
    }

    void toFileQueryResult(std::ofstream &toFileQueryFile, const std::shared_ptr<TNode> node1,
                           const std::shared_ptr<TNode> node2, const std::string select, const std::string relation,
                           const std::string param1,
                           const std::string param2,
                           const std::string synonym, const std::string value) {
        if (select == param1) {
            if (!synonym.empty()) {
                if (synonym == param1) {
                    if (node1->get_command_no() == std::stoi(value)) {
                        toFileQueryFile << "Node " << node2->get_command_no();
                        if (relation == "Follows") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Follows*") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Parent") {
                            toFileQueryFile << " parents ";
                        } else if (relation == "Parent*") {
                            toFileQueryFile << " parents* ";
                        } else if (relation == "Modifies") {
                            toFileQueryFile << " modifies ";
                        } else if (relation == "Uses") {
                            toFileQueryFile << " uses ";
                        }
                        toFileQueryFile << node1->
                                get_command_no() <<
                                "\n";
                    }
                } else if (synonym == param2) {
                    if (node2->get_command_no() == std::stoi(value)) {
                        toFileQueryFile << "Node " << node2->get_command_no();
                        if (relation == "Follows") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Follows*") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Parent") {
                            toFileQueryFile << " parents ";
                        } else if (relation == "Parent*") {
                            toFileQueryFile << " parents* ";
                        } else if (relation == "Modifies") {
                            toFileQueryFile << " modifies ";
                        } else if (relation == "Uses") {
                            toFileQueryFile << " uses ";
                        }
                        toFileQueryFile << node1->
                                get_command_no() <<
                                "\n";
                    }
                }
            } else {
                toFileQueryFile << "Node " << node2->get_command_no();
                if (relation == "Follows") {
                    toFileQueryFile << " follows ";
                } else if (relation == "Follows*") {
                    toFileQueryFile << " follows ";
                } else if (relation == "Parent") {
                    toFileQueryFile << " parents ";
                } else if (relation == "Parent*") {
                    toFileQueryFile << " parents* ";
                } else if (relation == "Modifies") {
                    toFileQueryFile << " modifies ";
                } else if (relation == "Uses") {
                    toFileQueryFile << " uses ";
                }
                toFileQueryFile << node1->
                        get_command_no() <<
                        "\n";
            }
        } else if (select == param2) {
            if (!synonym.empty()) {
                if (synonym == param1) {
                    if (node1->get_command_no() == std::stoi(value)) {
                        toFileQueryFile << "Node " << node2->get_command_no();
                        if (relation == "Follows") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Follows*") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Parent") {
                            toFileQueryFile << " parents ";
                        } else if (relation == "Parent*") {
                            toFileQueryFile << " parents* ";
                        } else if (relation == "Modifies") {
                            toFileQueryFile << " modifies ";
                        } else if (relation == "Uses") {
                            toFileQueryFile << " uses ";
                        }
                        toFileQueryFile << node1->
                                get_command_no() <<
                                "\n";
                    }
                } else if (synonym == param2) {
                    if (node2->get_command_no() == std::stoi(value)) {
                        toFileQueryFile << "Node " << node2->get_command_no();
                        if (relation == "Follows") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Follows*") {
                            toFileQueryFile << " follows ";
                        } else if (relation == "Parent") {
                            toFileQueryFile << " parents ";
                        } else if (relation == "Parent*") {
                            toFileQueryFile << " parents* ";
                        } else if (relation == "Modifies") {
                            toFileQueryFile << " modifies ";
                        } else if (relation == "Uses") {
                            toFileQueryFile << " uses ";
                        }
                        toFileQueryFile << node1->
                                get_command_no() <<
                                "\n";
                    }
                }
            } else {
                toFileQueryFile << "Node " << node2->get_command_no();
                if (relation == "Follows") {
                    toFileQueryFile << " follows ";
                } else if (relation == "Follows*") {
                    toFileQueryFile << " follows ";
                } else if (relation == "Parent") {
                    toFileQueryFile << " parents ";
                } else if (relation == "Parent*") {
                    toFileQueryFile << " parents* ";
                } else if (relation == "Modifies") {
                    toFileQueryFile << " modifies ";
                } else if (relation == "Uses") {
                    toFileQueryFile << " uses ";
                }
                toFileQueryFile << node1->
                        get_command_no() <<
                        "\n";
            }
        }
    }
}
