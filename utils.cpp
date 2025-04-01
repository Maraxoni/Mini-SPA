//
// Created by Kaspek on 01.04.2025.
//

#include "utils.h"

void fatal_error(const std::string &message) {
    std::cerr << "Fatal error: " << message << std::endl;

    throw std::runtime_error(message);
}
