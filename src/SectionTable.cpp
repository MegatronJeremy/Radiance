#include "../inc/SectionTable.hpp"

#include <iostream>

using namespace std;

SectionTable::SectionTable() {
    SectionDefinition und{};
    und.name = "UND";

    sectionDefinitions.emplace_back(und);
}

uint32_t SectionTable::insertSectionDefinition(SectionDefinition &sd) {
    if (currentSection != UND) {
        closeLastSection(sd.base);
    }

    sd.base += sectionDefinitions.back().base;
    sectionDefinitions.emplace_back(sd);

    return ++currentSection;
}

void SectionTable::closeLastSection(uint32_t endLocation) {
    SectionDefinition &prev = sectionDefinitions.back();
    prev.length = endLocation;
}

ostream &operator<<(ostream &os, const SectionDefinition &sd) {
    return os << sd.name << " " << sd.base << " " << sd.length;
}

ostream &operator<<(ostream &os, const SectionTable &st) {
    os << "Section table: " << endl;
    int i = 0;
    for (const SectionDefinition &sd: st.sectionDefinitions) {
        os << sd << " : " << i++ << endl;
    }
    return os;
}