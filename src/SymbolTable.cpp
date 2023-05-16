#include "../inc/SymbolTable.hpp"

#include <iostream>

using namespace std;

Elf32_Sym *SymbolTable::insertSymbolDefinition(Elf32_Sym &sd, const string &name) {
    cout << name << endl;
    symbolMappings[name] = sd.st_name = symbolDefinitions.size();
    symbolDefinitions.emplace_back(sd);
    return &symbolDefinitions.back();
}

ostream &operator<<(ostream &os, const Elf32_Sym &sd) {
    return os << sd.st_shndx << " " << sd.st_value << " " << ELF32_ST_BIND(sd.st_info) << " "
              << ELF32_ST_TYPE(sd.st_info);
}

ostream &operator<<(ostream &os, const SymbolTable &st) {
    os << "Symbol table: " << endl;
    for (const Elf32_Sym &sd: st.symbolDefinitions) {
        os << sd << ": " << sd.st_name << endl;
    }
    os << endl << "Symbol mappings: " << endl;
    for (auto &it: st.symbolMappings) {
        cout << it.first << " " << it.second << endl;
    }
    return os;
}

Elf32_Sym *SymbolTable::get(const string &s) {
    if (symbolMappings.find(s) == symbolMappings.end())
        return nullptr;

    return &symbolDefinitions[symbolMappings[s]];
}
