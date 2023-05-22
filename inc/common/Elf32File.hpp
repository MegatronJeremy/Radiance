#pragma once

#include "Elf32.hpp"
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <unordered_map>

using namespace std;

struct Elf32File {
public:
    explicit Elf32File(const string &fileName);

    void loadSection(const Elf32_Shdr &sh);

    string symbolName(const Elf32_Sym &sym) const {
        return stringTable[sym.st_name];
    }

    string sectionName(const Elf32_Shdr &shdr) const {
        return stringTable[shdr.sh_name];
    }

    string sectionName(const Elf32_Sym &sym) const {
        return sectionName(sectionHeaderTable[sym.st_shndx]);
    }

    string sectionName(Elf32_Section sec) const {
        return sectionName(sectionHeaderTable[sec]);
    }

    Elf32_Sym &sectionSymbol(Elf32_Section sec) {
        return symbolTable[sectionHeaderTable[sec].sh_name];
    }

    friend ostream &operator<<(ostream &os, const Elf32File &file);

    ~Elf32File();

    ifstream file;

    Elf32_Ehdr elfHeader;
    vector<vector<unsigned char>> dataSections;
    unordered_map<Elf32_Section, vector<Elf32_Rela>> relocationTables;
    vector<Elf32_Sym> symbolTable;
    vector<string> stringTable;
    vector<Elf32_Shdr> sectionHeaderTable;

};