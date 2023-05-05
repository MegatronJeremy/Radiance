#include "../inc/SymbolTable.hpp"

void SymbolTable::insertSymbolDefinition(SymbolDefinition &sd)
{
    symbolMappings[sd.name] = symbolDefinitions.size();
    symbolDefinitions.emplace_back(sd);
    lastAdded += 1;
}

ostream &operator<<(ostream &os, const SymbolDefinition &sd)
{
    return os << sd.name << " " << sd.section << " " << sd.symbolValue << " " << sd.globalDef;
}

ostream &operator<<(ostream &os, const SymbolTable &st)
{
    os << "Symbol table: " << endl;
    int i = 0;
    for (const SymbolDefinition &sd : st.symbolDefinitions)
    {
        os << sd << " : " << i++ << endl;
    }
    return os;
}