#include "../../inc/common/SectionTable.hpp"

#include <iostream>

using namespace std;


Elf32_Shdr *SectionTable::get(const string &s) {
    if (sectionIndices.find(s) == sectionIndices.end()) {
        return nullptr;
    }
    return &sectionDefinitions[sectionIndices[s]];
}

Elf32_Shdr &SectionTable::add(const Elf32_Shdr &shdr) {
    sectionDefinitions.emplace_back(shdr);
    return sectionDefinitions.back();
}

Elf32_Shdr &SectionTable::add(const Elf32_Shdr &shdr, const string &s) {
    sectionIndices[s] = sectionDefinitions.size(); // zero indexed
    add(shdr);
    return sectionDefinitions.back();
}

void SectionTable::closeLastSection(Elf32_Addr endLocation) {
    Elf32_Shdr &prev = sectionDefinitions.back();
    prev.sh_size = endLocation;
}

Elf32_Shdr &SectionTable::get(Elf32_Section s) {
    return sectionDefinitions[s];
}

const Elf32_Shdr &SectionTable::get(Elf32_Section s) const {
    return sectionDefinitions[s];
}

Elf32_Section SectionTable::getSectionIndex(const string &s) {
    return sectionIndices[s];
}