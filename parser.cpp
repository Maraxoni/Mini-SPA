#include "parser.h"

void parser::test1() {
    std::string path = "../files/input_min.txt";
    Parser parser;
    if (!parser.initialize_by_file(path)) {
        return;
    }

    auto procedure = parser.parseProcedure();
    std::cout << procedure->to_string() << std::endl;
}
