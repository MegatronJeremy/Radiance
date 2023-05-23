#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Elf32.hpp"

using namespace std;


struct SectionTable {
    Elf32_Shdr &get(Elf32_Section s);

    Elf32_Shdr *get(const string &s);

    Elf32_Section getSectionIndex(const string &s);

    const Elf32_Shdr &get(Elf32_Section s) const;

    Elf32_Shdr &add(const Elf32_Shdr &shdr = {});

    Elf32_Shdr &add(const Elf32_Shdr &shdr, const string &s);

    void closeLastSection(Elf32_Addr endLocation);

    friend ostream &operator<<(ostream &os, const SectionTable &st);

    vector<Elf32_Shdr> sectionDefinitions;

private:
    unordered_map<string, Elf32_Section> sectionIndices;

};