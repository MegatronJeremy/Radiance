#include "../inc/SectionTable.hpp"

#include <iostream>

using namespace std;

SectionTable::SectionTable() {
    Elf32_Shdr und{};

    sectionDefinitions.emplace_back(und);
}

void SectionTable::insertSectionDefinition(Elf32_Shdr sd, const string &name) {
    // size 1 - only undefined section present
    if (sectionDefinitions.size() != 1) {
        closeLastSection(sd.sh_addr);
    }

    sd.sh_addr += sectionDefinitions.back().sh_addr;
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