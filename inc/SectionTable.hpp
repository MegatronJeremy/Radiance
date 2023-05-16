#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Elf32.hpp"

using namespace std;


class SectionTable {
public:
    SectionTable();

    Elf32_Shdr &get(Elf32_Section s);

    void insertSectionDefinition(Elf32_Shdr sd, const string &name);

    void closeLastSection(Elf32_Addr endLocation);

    friend ostream &operator<<(ostream &os, const SectionTable &st);

private:
    vector<Elf32_Shdr> sectionDefinitions;
};