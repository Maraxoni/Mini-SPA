#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>
#include "pkb.h"

namespace query {
    // Query processing function
    void processQueries();

    void processRelation(std::ofstream &processRelationFile, const std::string &select, const std::string &relation,
                         const std::string &param1, const std::string &param2,
                         const std::string &synonym = "", const std::string &attribute = "",
                         const std::string &value = "");

    std::vector<std::shared_ptr<TNode> > getAllNodes(std::shared_ptr<TNode> currentRoot);
}

#endif //QUERY_H
