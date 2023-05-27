#include "../../inc/assembler/Assembler.hpp"

void Assembler::registerSection(const string &section) {
    if (pass == 1) {
        insertSection(section);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        currentSection++; // next section
        locationCounter = 0; // reset location counter
    }
}

void Assembler::insertSection(const string &section) {
    // open new literal table and relocation table for current section
    literalTable.emplace_back();

    currentSection++;

    // close last section
    eFile.sectionTable.closeLastSection(locationCounter);

    // reset location counter
    locationCounter = 0;

    // insert as section
    Elf32_Shdr &sd = eFile.sectionTable.add();
    sd.sh_type = SHT_PROGBITS;

    // insert section as symbol
    Elf32_Sym *sym = insertLocalSymbol(section);

    // section always has local type (for later symbol table lookup)
    sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
    // same string table index as symbol
    sd.sh_name = sym->st_name;
}
