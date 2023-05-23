#include "../../inc/common/SymbolTable.hpp"

#include <iostream>

using namespace std;

Elf32_Sym *SymbolTable::insertSymbolDefinition(Elf32_Sym &sd, const string &name) {
    symbolMappings[name] = sd.st_name = symbolDefinitions.size();

    symbolNames.emplace_back(name);

    symbolDefinitions.emplace_back(sd);

    return &symbolDefinitions.back();
}

Elf32_Word SymbolTable::getSymbolIndex(const string &s) {
    return symbolMappings[s];
}

Elf32_Sym *SymbolTable::get(const string &s) {
    if (symbolMappings.find(s) == symbolMappings.end())
        return nullptr;

    return &symbolDefinitions[symbolMappings[s]];
}

ostream &operator<<(ostream &os, const SymbolTable &st) {
    os << "Symbol table: " << endl;
    for (const Elf32_Sym &sd: st.symbolDefinitions) {
        os << sd << endl;
    }
    return os;
}

