#include "pkb.h"

void pkb::test() {
    const auto ast = PKB::build_AST();
    std::cout << ast->to_string() << std::endl;
}
