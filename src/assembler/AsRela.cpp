#include "../../inc/assembler/Assembler.hpp"

Elf32_Word Assembler::generateAbsoluteRelocation(const string &symbol) {
    // for generating R_32S
    Elf32_Sym *sd = eFile.symbolTable.get(symbol);

    Elf32_Rela rd;
    Elf32_Word relocationValue = 0;

    if (sd->st_shndx == SHN_ABS) {
        if (equExtRela.find(symbol) != equExtRela.end()) {
            // EQU symbol with external binding
            rd = equExtRela[symbol];
        } else {
            // fully absolute, no need for relocation
            relocationValue = sd->st_value;
            return relocationValue;
        }
    } else if (sd->st_shndx != SHN_UNDEF && ELF32_ST_BIND(sd->st_info) == STB_LOCAL) {
        // if symbol is local use the section index
        rd.r_addend = static_cast<Elf32_Sword>(sd->st_value);

        Elf32_Word sectionName = eFile.sectionTable.get(sd->st_shndx).sh_name;

        rd.r_info = ELF32_R_INFO(sectionName, R_32);
    } else {
        // if global or undefined use symbol index
        rd.r_addend = 0;
        rd.r_info = ELF32_R_INFO(sd->st_name, R_32);
    }

    rd.r_offset = locationCounter;

    auto &currentRelaTable = eFile.relocationTables[currentSection];
    currentRelaTable.insertRelocationEntry(rd);

    return relocationValue;
}
