#include <bits/stdc++.h>

//#include "parser.h"

#include "pkb.h"
#include "Query/query.h"

int main() {
    //std::string path = "../files/input_min.txt";
    std::string path = "../files/SIMPLE_Source_parser_test.txt";
    if (!Parser::instance().initialize_by_file(path)) {
        fatal_error(__PRETTY_FUNCTION__, __LINE__, "Failed to initialize parser");
        return -1;
    }
    //PKB::instance().initialize();

    int input=1;
    while(input!=0){
    std::cout << "1. Parser" << std::endl;
    std::cout << "2. PKB" << std::endl;
    std::cout << "3. Query" << std::endl;
    std::cout << "4. PKB::instance().initialize()" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Enter option: " << std::endl;
    std::cin >> input;

    switch (input) {
        case 1: {
            std::cout << "Parser: " << std::endl;
            parser::test1();
            break;
        }
        case 2: {
            std::cout << "PKB: " << std::endl;
            pkb::test();
            //
            break;
        }
        case 3: {
            std::cout << "Query: " << std::endl;
            query::processQueries();
            break;
        }
        case 4: {
            std::cout << "PKB::instance().initialize(): " << std::endl;
            PKB::instance().initialize();
            break;
        }
    }
    }
    return 0;
}
