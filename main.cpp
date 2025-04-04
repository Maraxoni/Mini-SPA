#include <bits/stdc++.h>

//#include "parser.h"
#include "pkb.h"

int main() {
    int input;
    std::cout << "1. Parser" << std::endl;
    std::cout << "2. PKB" << std::endl;
    std::cout << "3. Query" << std::endl;
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
            //
            break;
        }
    }

    return 0;
}