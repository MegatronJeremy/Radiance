#pragma once

#include "Elf32.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

using namespace std;

struct SymbolTable {
    Elf32_Sym *get(const string &s);

    Elf32_Sym *insertSymbolDefinition(Elf32_Sym &sd, const string &name);

    friend ostream &operator<<(ostream &os, const SymbolTable &);

    // strtab
    unordered_map<string, Elf32_Word> symbolMappings;

    vector<Elf32_Sym> symbolDefinitions;

    vector<string> symbolNames;

};
