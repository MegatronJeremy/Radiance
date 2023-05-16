#include "../inc/SectionTable.hpp"

#include <iostream>

using namespace std;

SectionTable::SectionTable() {
    Elf32_Shdr und{};

    sectionDefinitions.emplace_back(und);
}

void SectionTable::insertSectionDefinition(Elf32_Shdr sd, const string &name) {
    sectionDefinitions.emplace_back(sd);
}

void SectionTable::closeLastSection(Elf32_Addr endLocation) {
    Elf32_Shdr &prev = sectionDefinitions.back();
    prev.sh_size = endLocation;
}

ostream &operator<<(ostream &os, const Elf32_Shdr &sd) {
    return os << sd.sh_addr << " " << sd.sh_size;
}

ostream &operator<<(ostream &os, const SectionTable &st) {
    os << "Section table: " << endl;
    int i = 0;
    for (const Elf32_Shdr &sd: st.sectionDefinitions) {
        os << sd << ": " << i++ << endl;
    }
    return os;
}

Elf32_Shdr &SectionTable::get(Elf32_Section s) {
    return sectionDefinitions[s];
}
