//
// Created by Marax on 27.04.2025.
//

#ifndef SUBINSTRUCTION_H
#define SUBINSTRUCTION_H
#include <string>
#include <unordered_map>
#include <vector>

#include "SynonymConstraint.h"
namespace query {
    class SubInstruction {
    public:
        std::string relation;
        std::string left_param;
        std::string right_param;
        std::vector<SynonymConstraint > synonym_constraints;

        SubInstruction() = default;

        SubInstruction(const std::string &rel, const std::string &left_p, const std::string &right_p)
            : relation(rel), left_param(left_p), right_param(right_p) {
        }

        void add_synonym_constraint(const SynonymConstraint  &synonmym_constraint) {
            synonym_constraints.push_back(synonmym_constraint);
        }
    };
}

#endif //SUBINSTRUCTION_H
