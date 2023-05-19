#pragma once

#include "Elf32.hpp"
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <unordered_map>

using namespace std;

class Elf32File {
public:
    explicit Elf32File(const string &fileName);

    void loadSection(const Elf32_Shdr &sh);

    friend ostream &operator<<(ostream &os, const Elf32File &file);

private:
    ifstream file;

    Elf32_Ehdr elfHeader;
    vector<vector<Elf32_Word>> dataSections;
    unordered_map<Elf32_Section, vector<Elf32_Rela>> relocationTables;
    vector<Elf32_Sym> symbolTable;
    vector<string> stringTable;
    vector<Elf32_Shdr> sectionHeaderTable;

};