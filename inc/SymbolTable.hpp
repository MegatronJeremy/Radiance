#pragma once

#include "Constants.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

using namespace std;

struct SymbolDefinition {
    uint32_t symbolValue = 0;
    uint32_t section = UND;
    bool globalDef = false;
    string name;

    friend ostream &operator<<(ostream &os, const SymbolDefinition &);
};

class SymbolTable {
public:
    SymbolDefinition *get(const string &s) {
        if (symbolMappings.find(s) == symbolMappings.end())
            return nullptr;

        return &symbolDefinitions[symbolMappings[s]];
    }

    void insertSymbolDefinition(SymbolDefinition &);

    friend ostream &operator<<(ostream &os, const SymbolTable &);

private:
    uint32_t lastAdded = 0;

    unordered_map<string, uint32_t> symbolMappings;
    vector<SymbolDefinition> symbolDefinitions;
};
