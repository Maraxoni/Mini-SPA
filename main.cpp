#include <bits/stdc++.h>

#include "parser.h"

int main() {
    int input;
    std::cout << "1. Parser" << std::endl;
    std::cout << "Enter option: " << std::endl;
    std::cin >> input;

    switch (input) {
        case 1: {
            std::cout << "Parser: " << std::endl;
            parser::test1();
            break;
        }
    }

    return 0;
}