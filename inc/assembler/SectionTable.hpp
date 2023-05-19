#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../common/Elf32.hpp"

using namespace std;


struct SectionTable {
    Elf32_Shdr &get(Elf32_Section s);

    Elf32_Shdr &insertSectionDefinition(Elf32_Shdr sd = {});

    void closeLastSection(Elf32_Addr endLocation);

    friend ostream &operator<<(ostream &os, const SectionTable &st);

    vector<Elf32_Shdr> sectionDefinitions;
};