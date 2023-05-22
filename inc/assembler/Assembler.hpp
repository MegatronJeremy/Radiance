#pragma once

#include <fstream>
#include "SectionTable.hpp"
#include "../common/SymbolTable.hpp"
#include "RelocationTable.hpp"
#include "LiteralTable.hpp"
#include "../common/Ins32.hpp"
#include "../../misc/parser.hpp"

class Assembler {
public:
    explicit Assembler(string output);

    Elf32_Sym *insertLocalSymbol(const string &symbol);

    void insertAbsoluteSymbol(const string &symbol, Elf32_Addr symbolValue);

    void insertGlobalSymbol(const string &symbol);

    Elf32_Addr getPoolConstantAddr(const PoolConstant &constant);

    void initSpaceWithConstant(const string &symbol);

    void initSpaceWithConstant(Elf32_Word literal);

    void initAscii(string ascii);

    void zeroInitSpace(Elf32_Word bytes);

    void registerSection(const string &section);

    void insertSection(const string &section);

    void addSymbolUsage(const string &symbol);

    void insertInstruction(yytokentype token, const vector<int16_t> &fields = {});

    void insertFlowControlIns(yytokentype type, const PoolConstant &constant,
                              const vector<int16_t> &fields = {R0, R0, R0});


    void insertLoadIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields);

    void insertStoreIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields);

    void insertIretIns();

    void generateAbsoluteRelocation(const string &symbol);

    void generateRelativeRelocation(Elf32_Sym *sd);

    void endAssembly();

    void initCurrentSectionPoolConstants();

    bool nextPass();

    void prepareSecondPass();

    void writeSymbolTable(vector<Elf32_Shdr> &additionalHeaders);

    void writeStringTable(vector<Elf32_Shdr> &additionalHeaders);

    void writeRelocationTables(vector<Elf32_Shdr> &additionalHeaders);

    void writeToOutputFile();

    static void incLocationCounter(Elf32_Word bytes = 4);

    friend ostream &operator<<(ostream &os, const Assembler &as);


    ~Assembler() {
        outputFile.close();
    }

private:
    Elf32_Ehdr elfHeader;

    ofstream outputFile;

    SectionTable sectionTable;
    SymbolTable symbolTable;

    // one for each section
    vector<RelocationTable> relocationTable;
    vector<LiteralTable> literalTable;

    uint32_t currentSection = SHN_UNDEF;
    static Elf32_Addr locationCounter;

    string outFile;

    int pass = 0;
};