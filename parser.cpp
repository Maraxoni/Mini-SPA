#include "parser.h"

void parser::test1() {
//    auto procedure = Parser::instance().parse_procedure();
//    std::cout << procedure->to_string() << std::endl;

    auto& parser = Parser::instance();
    parser.parse_program();
    const auto& all_procedures = parser.get_all_procedures();

    std::cout << "=== Parsed Procedures ===\n";
    for (const auto& [name, proc] : all_procedures) {
        proc->print();
    }

}
