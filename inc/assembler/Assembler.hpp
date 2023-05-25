#pragma once

#include <fstream>
#include "../common/SectionTable.hpp"
#include "../common/SymbolTable.hpp"
#include "../common/RelocationTable.hpp"
#include "../common/LiteralTable.hpp"
#include "../common/Ins32.hpp"
#include "../../misc/parser.hpp"
#include "../common/Elf32File.hpp"

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

    void insertJumpIns(yytokentype type, const PoolConstant &constant,
                       const vector<int16_t> &fields = {R0, R0, R0});

    void insertCallIns(const PoolConstant &constant);


    void insertLoadIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields);

    void insertStoreIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields);

    void insertIretIns();

    Elf32_Word generateAbsoluteRelocation(const string &symbol);

    void endAssembly();

    void initCurrentSectionPoolConstants();

    bool nextPass();

    static void incLocationCounter(Elf32_Word bytes = 4);

private:
    Elf32_Ehdr elfHeader;

    // output file wrapper
    Elf32File eFile;

    // one for each section
    vector<LiteralTable> literalTable;

    uint32_t currentSection = SHN_UNDEF;
    static Elf32_Addr locationCounter;

    string outFile;

    int pass = 0;
};