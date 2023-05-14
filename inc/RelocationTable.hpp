#pragma once

#include <vector>

#include "Elf32.hpp"

using namespace std;

class RelocationTable {
public:
    void insertRelocationEntry(Elf32_Rela rd);

    friend ostream &operator<<(ostream &os, const RelocationTable &rt);

private:
    vector<Elf32_Rela> relocationEntries;
};


