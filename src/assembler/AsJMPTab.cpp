#include "../../inc/assembler/Assembler.hpp"

#include <algorithm>

void Assembler::resolveJMPTab() {
    for (auto &it: jmpTabs) {
        Elf32_Section sec = it.first;
        vector<JMPTabEntry> &jmpTab = it.second;

        sort(jmpTab.begin(), jmpTab.end(), [](JMPTabEntry &a, JMPTabEntry &b) { return a.address > b.address; });

        for (auto &ent: jmpTab) {
            Elf32_Sym *sym = eFile.symbolTable.get(ent.symbol);
            if (sym->st_shndx != sec) { // ned to increase values of all symbols
                Elf32_Shdr &section = eFile.sectionTable.get(sec);
                section.sh_size += ent.pad;
                increaseAddresses(sec, ent.address, ent.pad);
            }
        }
    }
}

void Assembler::increaseAddresses(Elf32_Section sec, Elf32_Word address, Elf32_Word pad) {
    for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
        if (sym.st_shndx != sec || sym.st_value <= address) {
            continue;
        }
        sym.st_value += pad;
    }
}

