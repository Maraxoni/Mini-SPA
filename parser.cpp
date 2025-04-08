#include "parser.h"

void parser::test1() {
    auto procedure = Parser::instance().parse_procedure();
    std::cout << procedure->to_string() << std::endl;
}
