#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>
#include "pkb.h"

namespace query {
    // Query processing function
    void processQueries();

    void processRelation(std::ofstream &processRelationFile, const std::string &select,
                         const std::string &relation,
                         const std::string &param1, const std::string &param2,
                         const std::string &synonym = "", const std::string &attribute = "",
                         const std::string &value = "");

    void query_check_static_numbers(std::ofstream &toFileQueryFile, const std::shared_ptr<TNode> &node1,
                           const std::shared_ptr<TNode> &node2, const std::string &select, const std::string &relation,
                           const std::string &param1,
                           const std::string &param2,
                           const std::string &synonym, const std::string &value);

    void query_check_with(std::ofstream &toFileQueryFile, const std::shared_ptr<TNode> &node1,
                           const std::shared_ptr<TNode> &node2, const std::string &select, const std::string &relation,
                           const std::string &param1,
                           const std::string &param2,
                           const std::string &synonym, const std::string &value);

    bool query_is_number(const std::string &str);
    void to_file_param1(std::ofstream &toFileParam1, const std::shared_ptr<TNode> &node1,
                        const std::shared_ptr<TNode> &node2, const std::string &relation);
    void to_file_param2(std::ofstream &toFileParam2, const std::shared_ptr<TNode> &node1,
                           const std::shared_ptr<TNode> &node2, const std::string &relation);
}

#endif //QUERY_H
