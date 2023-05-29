#pragma once

#include "Elf32.hpp"
#include "Ins32.hpp"
#include <cmath>

class Util {
public:
    static bool valueFitsInDisp(Elf32_Sword value) {
        return value <= MAX_DISP && value >= MIN_DISP;
    }

    static bool positiveValueFitsInDisp(Elf32_Word value) {
        return value <= MAX_DISP;
    }

    static int16_t getDisplacement(Elf32_Addr from, Elf32_Addr to);
};