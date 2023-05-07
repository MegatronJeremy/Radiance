#pragma once

#include "SectionTable.hpp"
#include "SymbolTable.hpp"

#include "Constants.hpp"

class Assembler {
public:
    void insertLocalSymbol(const string &symbol);

    void insertAbsoluteSymbol(const string &symbol, uint32_t symbolValue);

    void insertGlobalSymbol(const string &symbol);

    void registerSection(const string &section);

    void insertSection(const string &section);

    void reserveSpace(const string &str);

    void addSymbolUsage(const string &symbol);

    void endAssembly();

    bool nextPass();

    friend ostream &operator<<(ostream &os, const Assembler &as);

    uint32_t locationCounter = 0;

private:
    SectionTable sectionTable;
    SymbolTable symbolTable;

    uint32_t currentSection = UND;

    int pass = 0;
};