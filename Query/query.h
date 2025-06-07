#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>
#include "../pkb.h"

namespace query {
    // Query processing function
    std::string processPQL(const std::string& declarations, const std::string& selectLine, bool instruction_cond);
    void processQueries();
    void print_relations();
    std::string trim(const std::string& s);
}

#endif //QUERY_H
