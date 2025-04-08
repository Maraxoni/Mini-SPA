#include "pkb.h"

void pkb::test() {
    const auto ast = PKB::instance().get_ast();
    std::cout << ast->to_string() << std::endl;
}
