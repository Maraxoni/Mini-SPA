#include "pkb.h"

void pkb::test() {
    std::string path = "../files/input_min.txt";
    auto parser = std::make_shared<Parser>();
    if (!parser->initialize_by_file(path)) {
        return;
    }

    PKB pkb = PKB(parser);
    auto ast = pkb.build_AST();

    auto asdsad = pkb.get_ast_as_list(ast);

    std::shared_ptr<TNode> tmp = ast;
    while (tmp->get_first_child() != nullptr) {
        tmp = tmp->get_first_child();
    }
}
