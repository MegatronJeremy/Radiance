#pragma once

#include "Elf32.hpp"
#include <cmath>

bool valueFitsInDisp(Elf32_Sword value) {
    return value <= MAX_DISP && value >= MIN_DISP;
}

bool positiveValueFitsInDisp(Elf32_Word value) {
    return value <= MAX_DISP;
}

int16_t getDisplacement(Elf32_Addr from, Elf32_Addr to) {
    auto disp = static_cast<Elf32_Sword>(from - to);

    if (!valueFitsInDisp(disp)) {
        throw runtime_error("Assembler error: text segment is too large, displacement cannot fit into 12 bits!");
    }

    return static_cast<int16_t>(disp);
}