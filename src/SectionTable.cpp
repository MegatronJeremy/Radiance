#include "../inc/SectionTable.hpp"

#include <iostream>

using namespace std;


Elf32_Shdr &SectionTable::insertSectionDefinition(Elf32_Shdr sd) {
    sectionDefinitions.emplace_back(sd);
    return sectionDefinitions.back();
}

void SectionTable::closeLastSection(Elf32_Addr endLocation) {
    Elf32_Shdr &prev = sectionDefinitions.back();
    prev.sh_size = endLocation;
}

Elf32_Shdr &SectionTable::get(Elf32_Section s) {
    return sectionDefinitions[s];
}

ostream &operator<<(ostream &os, const SectionTable &st) {
    os << "Section table: " << endl;
    int i = 0;
    for (const Elf32_Shdr &sd: st.sectionDefinitions) {
        os << sd << ": " << i++ << endl;
    }
    return os;
}
