#include "../inc/SymbolTable.hpp"

#include <iostream>

using namespace std;

Elf32_Sym *SymbolTable::insertSymbolDefinition(Elf32_Sym &sd, const string &name) {
    cout << name << endl;
    symbolMappings[name] = sd.st_name = symbolDefinitions.size();

    symbolNames.emplace_back(name);

    symbolDefinitions.emplace_back(sd);

    return &symbolDefinitions.back();
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
    os << endl << "Symbol mappings: " << endl;
    for (auto &it: st.symbolMappings) {
        cout << it.first << " " << it.second << endl;
    }
    return os;
}
