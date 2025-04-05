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
                std::cout << "  Select: " << match[1] << "\n";
                std::cout << "  Relation: " << match[2] << "\n";
                std::cout << "  Param1: " << match[3] << "\n";
                std::cout << "  Param2: " << match[4] << "\n";
                if (match[5].matched) {
                    std::cout << "  With synonym: " << match[5] << "\n";
                    std::cout << "  Attribute: " << match[6] << "\n";
                    std::cout << "  Value: ";
                    if (match[7].matched) std::cout << "\"" << match[7] << "\"\n"; // tekst
                    else if (match[8].matched) std::cout << match[8] << "\n"; // liczba
                    else if (match[9].matched) std::cout << match[9] << "\n"; // identyfikator
                }
            } else {
                std::cout << "No match: " << line << "\n";
            }
        }

        // Close files
        inputFile.close();
        outputFile.close();

        std::cout << "Processing complete. Files saved." << outputFilePath << std::endl;
    }
}
