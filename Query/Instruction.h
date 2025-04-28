//
// Created by Marax on 27.04.2025.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "SubInstruction.h"

namespace query {
    class Instruction {
    public:
        std::string select;
        std::vector<SubInstruction> subInstructions;
        std::vector<std::vector<int> > results;

        Instruction(const std::string &sel) : select(sel) {
        }

        void addSubInstruction(const SubInstruction &sub) {
            subInstructions.push_back(sub);
        }

        void printInstruction() const {
            std::cout << "Select " << select << " such that ";
            bool first_sub_i = true;
            for (const auto &sub: subInstructions) {
                if (first_sub_i) {
                    first_sub_i = false;
                } else {
                    std::cout << " and ";
                }
                std::cout << sub.relation << " (" << sub.param1 << ", " << sub.param2 << ")";
                if (!sub.synonym.empty()) {
                    std::cout << " with " << sub.synonym << "." << sub.attribute << "= " << sub.value;
                }
            }
            std::cout << std::endl;
        }

        void printResults() const {
            std::cout << "Results for: " << select << std::endl;
        }

        void searchDatabase() {
            std::cout << "Searching parsed code for: " << select << std::endl;

            for (const auto &sub: subInstructions) {
            }
        }
    };
} // query

#endif //INSTRUCTION_H
