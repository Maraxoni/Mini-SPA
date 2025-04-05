#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>

namespace query {
    // Query processing function
    void processQueries();

    void processRelation(const std::string &select, const std::string &relation,
                         const std::string &param1, const std::string &param2,
                         const std::string &synonym = "", const std::string &attribute = "",
                         const std::string &value = "");
}

#endif //QUERY_H
