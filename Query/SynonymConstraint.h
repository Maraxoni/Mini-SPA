//
// Created by Marax on 19.05.2025.
//

#ifndef SYNONYMCONSTRAINT_H
#define SYNONYMCONSTRAINT_H
#include <string>
#include <unordered_map>
#include <vector>

#include "../pkb.h"

namespace query {
    class SynonymConstraint {
    public:
        std::string synonym;
        std::string attribute;
        std::string value;

        SynonymConstraint() = default;

        SynonymConstraint(const std::string& syn,
                          const std::string& attr,
                          const std::string& val)
            : synonym(syn), attribute(attr), value(val) {}
    };
}

#endif //SYNONYMCONSTRAINT_H
