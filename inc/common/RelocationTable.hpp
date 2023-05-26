#pragma once

#include <vector>

#include "Elf32.hpp"

using namespace std;

struct RelocationTable {
    void insertRelocationEntry(Elf32_Rela rd) {
        relocationEntries.emplace_back(rd);
    }

    vector<Elf32_Rela> relocationEntries;
};


