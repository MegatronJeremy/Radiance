#pragma once

#include "../RelocationTable.hpp"
#include "../SectionTable.hpp"
#include "../Elf32.hpp"
#include "../SymbolTable.hpp"
#include "../ProgramTable.hpp"
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <unordered_map>
#include <sstream>

using namespace std;

struct Elf32File {
public:
    void addUndefinedSym();

    void loadFromInputFile(const string &fileName);

    void writeRelToOutputFile(const string &fileName);

    void writeExecToOutputFile(const string &filename);

    string symbolName(const Elf32_Sym &sym) const {
        return symbolTable.symbolNames[sym.st_name];
    }

    string sectionName(const Elf32_Shdr &shdr) const {
        return symbolTable.symbolNames[shdr.sh_name];
    }

    string sectionName(const Elf32_Sym &sym) const {
        return sectionName(sectionTable.sectionDefinitions[sym.st_shndx]);
    }

    string sectionName(Elf32_Section sec) const {
        return sectionName(sectionTable.sectionDefinitions[sec]);
    }

    Elf32_Sym &sectionSymbol(Elf32_Section sec) {
        return symbolTable.symbolDefinitions[sectionTable.sectionDefinitions[sec].sh_name];
    }

    friend ostream &operator<<(ostream &os, Elf32File &file);

    Elf32_Ehdr elfHeader;
    unordered_map<Elf32_Section, stringstream> dataSections;
    unordered_map<Elf32_Section, RelocationTable> relocationTables;
    SymbolTable symbolTable;
    SectionTable sectionTable;
    ProgramTable programTable;

private:
    void loadRelFile(fstream &file);

    void loadExecFile(fstream &file);

    void loadSection(const Elf32_Shdr &sh, Elf32_Section &currDataSection, fstream &file);

    void loadSection(const Elf32_Phdr &ph, Elf32_Section &currDataSection, fstream &file);

    void writeStringTable(vector<Elf32_Shdr> &additionalHeaders, fstream &file);

    void writeSymbolTable(vector<Elf32_Shdr> &additionalHeaders, fstream &file);

    void writeRelocationTables(vector<Elf32_Shdr> &additionalHeaders, fstream &file);

    void writeRelDataSections(fstream &file);

    void writeExecDataSections(fstream &file);
};