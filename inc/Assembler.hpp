#pragma once

#include "SectionTable.hpp"
#include "SymbolTable.hpp"

#include "Constants.hpp"

class Assembler
{
  public:
    void insertLocalSymbol(string symbol);
    void insertAbsoluteSymbol(string symbol, uint32_t symbolValue);
    void insertGlobalSymbol(string symbol);
    void registerSection(string section);
    void insertSection(string section);
    void reserveSpace(string str);
    void addSymbolUsage(string symbol);
    void endAssembly();

    int locationCounter = 0;

    bool nextPass()
    {
        if (pass == 2)
            return false;

        currentSection = UND;
        locationCounter = 0;
        pass = pass + 1;
        return true;
    }

    friend ostream &operator<<(ostream &os, const Assembler &as);

  private:
    SectionTable sectionTable;
    SymbolTable symbolTable;

    uint32_t currentSection = UND;

    int pass = 0;
};