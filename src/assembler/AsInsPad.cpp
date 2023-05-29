#include "../../inc/assembler/Assembler.hpp"

#include <algorithm>

void Assembler::resolveIpadTab() {
    for (auto &it: ipadTabs) {
        Elf32_Section sec = it.first;
        vector<IpadTabEntry> &ipadTab = it.second;

        sort(ipadTab.begin(), ipadTab.end(), [](IpadTabEntry &a, IpadTabEntry &b) { return a.address > b.address; });

        for (auto &ent: ipadTab) {
            Elf32_Sym *sym = eFile.symbolTable.get(ent.symbol);
            if (sym->st_shndx != sec) { // need to increase values of all symbols
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

