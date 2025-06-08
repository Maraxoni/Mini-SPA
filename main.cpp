#include <iostream>
#include <string>
#include <fstream>

#include "pkb.h"
#include "Query/query.h"
#include "benchmark_tool.h"
#include "parser.h"

// Function to run the legacy test mode
void run_old_menu_mode() {
    std::string sourcePath = "../files/SIMPLE_Source_parser_test.txt";
    //std::string sourcePath = "../files/input_min.txt";
    std::string queryInputPath = "../files/query_input2.txt";
    std::string queryOutputPath = "../files/query_output.txt";

    Parser::instance().initialize_by_file(sourcePath);
    BenchmarkTool tool;
    Parser::instance().parse_program();
    tool.breakpoint(1);
    PKB::instance().initialize();
    tool.breakpoint(2);
    tool.reset();

    int input = 1;
    while (input != 0) {
        std::cout << "1. Parser" << std::endl;
        std::cout << "2. PKB" << std::endl;
        std::cout << "3. Query (process queries from file)" << std::endl;
        std::cout << "4. Re-initialize PKB" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter option: ";
        std::cin >> input;

        switch (input) {
            case 1:
                std::cout << "Running parser test..." << std::endl;
                parser::test1();
                break;
            case 2:
                std::cout << "Running PKB test..." << std::endl;
                pkb::test();
                break;
            case 3: {
                std::ifstream infile(queryInputPath);
                if (!infile.is_open()) {
                    std::cerr << "Error: Cannot open input file: " << queryInputPath << std::endl;
                    break;
                }

                std::ofstream outfile(queryOutputPath);
                if (!outfile.is_open()) {
                    std::cerr << "Error: Cannot create output file: " << queryOutputPath << std::endl;
                    break;
                }

                std::string declarations, selectLine;
                int queryNum = 1;

                while (std::getline(infile, declarations) && std::getline(infile, selectLine)) {

                    std::string result = query::processPQL(declarations, selectLine, true);
                    outfile << result << "\n";
                }

                infile.close();
                outfile.close();

                query::print_relations();

                std::cout << "Query results written to: " << queryOutputPath << std::endl;
                break;
            }
            case 4:
                std::cout << "Reinitializing PKB..." << std::endl;
                PKB::instance().initialize();
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments provided - run test mode
        run_old_menu_mode();
        return 0;
    } else if (argc == 2) {
        // Production mode for PipeTester
        std::string path = argv[1];
        try {
            if (!Parser::instance().initialize_by_file(path)) {
                std::cerr << "# Failed to initialize parser with file: " << path << std::endl;
                return -1;
            }

            BenchmarkTool tool;
            Parser::instance().parse_program();
            tool.breakpoint(1);
            PKB::instance().initialize();
            tool.breakpoint(2);
            tool.reset();

            std::cout << "Ready" << std::endl;
            std::cout.flush();

            std::string declarations, query;

            while (true) {
                if (!std::getline(std::cin, declarations)) break;
                if (!std::getline(std::cin, query)) break;

                try {
                    std::string response = query::processPQL(declarations, query, false);
                    std::cout << response << std::endl;
                    std::cout.flush();
                } catch (const std::exception& e) {
                    std::cout << "#" << e.what() << std::endl;
                    std::cout.flush();
                } catch (...) {
                    std::cout << "# Unknown error during query processing" << std::endl;
                    std::cout.flush();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "# Exception: " << e.what() << std::endl;
            return -1;
        }
        return 0;
    } else {
        std::cerr << "# Invalid number of arguments. Usage:" << std::endl;
        std::cerr << "# spa.exe <path_to_source.txt>" << std::endl;
        return -1;
    }
}
