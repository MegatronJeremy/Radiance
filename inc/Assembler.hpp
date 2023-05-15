#pragma once

#include <fstream>
#include "SectionTable.hpp"
#include "SymbolTable.hpp"
#include "RelocationTable.hpp"
#include "LiteralTable.hpp"
#include "../misc/parser.hpp"

class Assembler {
public:
    Assembler();

    Elf32_Sym *insertLocalSymbol(const string &symbol);

    void insertAbsoluteSymbol(const string &symbol, Elf32_Addr symbolValue);

    void insertGlobalSymbol(const string &symbol);

    void insertLiteral(const string &symbol);

    void insertLiteral(Elf32_Word literal);

    void initAscii(string ascii);

    void zeroInitSpace(Elf32_Word bytes);

    void incLocationCounter(Elf32_Word bytes = 4);

    void registerSection(const string &section);

    void insertSection(const string &section);

    void addSymbolUsage(const string &symbol);

    void insertInstruction(yytokentype token, const vector<int16_t> &fields = {});

    Elf32_Word generateAbsoluteRelocation(const string &symbol);

    Elf32_Word generateRelativeRelocation(const string &symbol);

    void endAssembly();

    bool nextPass();

    void prepareSecondPass();


    friend ostream &operator<<(ostream &os, const Assembler &as);


    ~Assembler() {
        cout << *this << endl;
        outputFile.close();
    }

private:
    Elf32_Ehdr elfHeader;

    ofstream outputFile;

    SectionTable sectionTable;
    SymbolTable symbolTable;
    RelocationTable relocationTable;
    vector<LiteralTable> literalTable;

    uint32_t currentSection = SHN_UNDEF;
    uint32_t locationCounter = 0;

    int pass = 0;
};