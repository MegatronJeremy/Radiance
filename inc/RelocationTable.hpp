#pragma once

#include <vector>

#include "Elf32.hpp"

using namespace std;

struct RelocationTable {
    void insertRelocationEntry(Elf32_Rela rd);

    friend ostream &operator<<(ostream &os, const RelocationTable &rt);

    vector<Elf32_Rela> relocationEntries;
};


