#include "query.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

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
                processRelation(select, relation, param1, param2, synonym, attribute, value);
            } else {
                std::cout << "No match: " << line << "\n";
            }
        }

        // Close files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }

    void processRelation(const std::string &select, const std::string &relation,
                         const std::string &param1, const std::string &param2,
                         const std::string &synonym, const std::string &attribute,
                         const std::string &value) {
        std::cout << "Processing Relation:\n";
        std::cout << "  Select: " << select << "\n";
        std::cout << "  Relation: " << relation << "\n";
        std::cout << "  Param1: " << param1 << "\n";
        std::cout << "  Param2: " << param2 << "\n";

        if (relation == "Follows") {
            std::cout << "Relation type: " << relation << "\n";
            //Follows();
        } else if (relation == "Follow*") {
            std::cout << "Relation type: " << relation << "\n";
            //Follow*();
        } else if (relation == "Parent") {
            std::cout << "Relation type: " << relation << "\n";
            //Parent();
        } else if (relation == "Parent*") {
            std::cout << "Relation type: " << relation << "\n";
            //Parent*();
        } else if (relation == "Modifies") {
            std::cout << "Relation type: " << relation << "\n";
            //Modifies();
        } else if (relation == "Uses") {
            std::cout << "Relation type: " << relation << "\n";
            //Uses();
        } else {
            std::cout << "Unknown relation type\n";
        }

        // Additional info for "with"
        if (!synonym.empty()) {
            std::cout << "  With synonym: " << synonym << "\n";
            std::cout << "  Attribute: " << attribute << "\n";
            std::cout << "  Value: " << value << "\n";
        }
    }
}
