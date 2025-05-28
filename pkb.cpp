#include "pkb.h"

std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::usesRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::modifiesRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::followsTRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::followsRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::parentTRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::parentRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::callsRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::nextRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();
std::shared_ptr<std::vector<std::pair<TNode, TNode>>> PKB::nextTRelations = std::make_shared<std::vector<std::pair<TNode, TNode>>>();

void pkb::test() {
    const auto ast = PKB::instance().get_ast();
    std::cout << ast->to_string() << std::endl;
}