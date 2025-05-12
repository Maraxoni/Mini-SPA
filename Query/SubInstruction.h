//
// Created by Marax on 27.04.2025.
//

#ifndef SUBINSTRUCTION_H
#define SUBINSTRUCTION_H
#include <string>
#include <unordered_map>
#include <vector>

#include "../pkb.h"

class SubInstruction {
public:
    std::string relation;
    std::string param1;
    std::string param2;
    std::string synonym;
    std::string attribute;
    std::string value;
    std::unordered_map<std::shared_ptr<TNode>, std::vector<std::shared_ptr<TNode>>> resultsMap;

    SubInstruction() = default;

    SubInstruction(const std::string &rel, const std::string &p1, const std::string &p2, const std::string &syn = "",
                   const std::string &attr = "", const std::string &val = "")
        : relation(rel), param1(p1), param2(p2), synonym(syn), attribute(attr), value(val) {
    }
};


#endif //SUBINSTRUCTION_H
