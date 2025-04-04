#include "pkb.h"

void pkb::test() {
    std::string path = "../files/input_min.txt";
    auto parser = std::make_shared<Parser>();
    if (!parser->initialize_by_file(path)) {
        return;
    }

    PKB pkb = PKB(parser);
    auto ast = pkb.build_AST();

    std::shared_ptr<TNode> tmp = ast;
    while (tmp->get_first_child() != nullptr) {
        tmp = tmp->get_first_child();
    }

    std::cout << ast->to_string() << std::endl;

    std::cout << pkb.follows(ast->get_first_child(), ast->get_first_child()->get_right_sibling()) << std::endl;
}
