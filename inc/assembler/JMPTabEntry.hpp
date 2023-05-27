#pragma once

#include "../common/Elf32.hpp"

#include <string>

struct JMPTabEntry {
    Elf32_Word address;
    Elf32_Word pad; // pad -> if not in the same section
    std::string symbol;
};