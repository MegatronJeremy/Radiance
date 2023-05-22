#pragma once

#include <string>
#include <vector>

#include "Elf32.hpp"

using namespace std;


struct ProgramTable {
    Elf32_Phdr &get(Elf32_Section s) {
        return programDefinitions[s - 1];
    }

    const Elf32_Phdr &get(Elf32_Section s) const {
        return programDefinitions[s - 1];
    }

    void add(const Elf32_Phdr &ph) {
        programDefinitions.emplace_back(ph);
    }

    friend ostream &operator<<(ostream &os, const ProgramTable &pt);

    vector<Elf32_Phdr> programDefinitions;
};
