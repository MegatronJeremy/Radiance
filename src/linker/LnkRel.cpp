#include "../../inc/linker/Linker.hpp"

void Linker::generateRelocations(Elf32File &eFile) {
    for (auto &it: eFile.relocationTables) {
        Elf32_Shdr &sh = eFile.sectionTable.get(it.first);
        string secName = eFile.sectionName(sh);
        for (Elf32_Rela &rel: it.second.relocationEntries) {
            string symName = eFile.symbolTable.symbolNames[ELF32_R_SYM(rel.r_info)];

            // symbol in out file
            Elf32_Sym *outSym = outFile.symbolTable.get(symName);

            Elf32_Rela outRela = rel;
            outRela.r_offset += sh.sh_addr; // add new section start offset (from zero)
            outRela.r_info = ELF32_R_INFO(outSym->st_name, ELF32_R_TYPE(rel.r_info));

            Elf32_Section shndx = outFile.sectionTable.getSectionIndex(secName);

            outFile.relocationTables[shndx].insertRelocationEntry(outRela);
        }
    }
}

void Linker::handleRelocations(Elf32File &eFile) {
    for (auto &it: eFile.relocationTables) {
        Elf32_Section sec = it.first;
        Elf32_Sym &secSym = eFile.sectionSymbol(sec);
        stringstream &secData = eFile.dataSections[sec];
        for (auto &rel: it.second.relocationEntries) {
            Elf32_Sym &sym = eFile.symbolTable.symbolDefinitions[ELF32_R_SYM(rel.r_info)];
            string symName = eFile.symbolName(sym);

            Elf32_Sym *outSym = outFile.symbolTable.get(symName);
            Elf32_Addr relVal;

            switch (ELF32_R_TYPE(rel.r_info)) {
                case R_32S:
                case R_32:
                    // absolute relocation: S + A
                    relVal = outSym->st_value + rel.r_addend;
                    secData.seekp(rel.r_offset);
                    break;
                case R_PC32:
                    // relative relocation: S - P + A
                    relVal = outSym->st_value - (rel.r_offset + secSym.st_value) + rel.r_addend;
                    secData.seekp(rel.r_offset);
                    break;
                default:
                    throw runtime_error("Linker error: unrecognized relocation type");
            }

            secData.write(reinterpret_cast<char *>(&relVal), sizeof(Elf32_Addr));
        }
    }
}
