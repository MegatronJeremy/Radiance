#include "../../inc/linker/Linker.hpp"

#include <unordered_set>

void Linker::defineAllSymbols() {
    unordered_set<string> undefinedDefs;
    unordered_set<string> globalDefs;

    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
            if (sym.st_name == SHN_UNDEF) {
                // nothing to do - symbol value is in undefined section
                continue;
            }

            string symName = eFile.symbolName(sym);
            if (sym.st_shndx == SHN_UNDEF && globalDefs.count(symName) == 0) {
                // undefined weak symbol, add to U
                undefinedDefs.insert(symName);
            } else if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL) {
                // defined global symbol
                if (globalDefs.find(symName) != globalDefs.end()) {
                    throw runtime_error("Linker error: multiple definitions of symbol: " + symName);
                }

                // add sym to global defs
                globalDefs.insert(symName);

                // and erase from undefined symbols
                undefinedDefs.erase(symName);
            }

            if (sym.st_shndx != SHN_ABS &&
                (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL || ELF32_ST_TYPE(sym.st_info) == STT_SECTION)) {
                // subsection offset + address of new section
                string sectionName = eFile.sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;
            }
        }
    }
    if (execMode && !undefinedDefs.empty()) {
        string s = "Linker error: symbols without a definition: ";
        for (auto &it: undefinedDefs) {
            s += it + " ";
        }
        throw runtime_error(s);
    }

}

void Linker::generateSymbols() {
    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
            string symName = eFile.symbolName(sym);

            if ((ELF32_ST_TYPE(sym.st_info) != STT_SECTION && ELF32_ST_BIND(sym.st_info) != STB_GLOBAL) ||
                sym.st_shndx == SHN_UNDEF) {
                continue; // symbol defined in other section or symbol is not global
            }

            Elf32_Sym nsym = sym;

            // only if section exists
            if (nsym.st_shndx != SHN_ABS && nsym.st_shndx != SHN_UNDEF) {
                string sectionName = eFile.sectionName(sym);
                nsym.st_shndx = outFile.sectionTable.getSectionIndex(sectionName);
            }

            outFile.symbolTable.insertSymbolDefinition(nsym, symName);

            if (ELF32_ST_TYPE(nsym.st_info) == STT_SECTION) {
                // section symbol, add name index to section table
                outFile.sectionTable.get(symName)->sh_name = outFile.symbolTable.getSymbolIndex(symName);
            }
        }
    }
}
