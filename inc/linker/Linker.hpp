#pragma once

#include <string>
#include <fstream>
#include <utility>
#include <memory>

#include "../../inc/common/Elf32File.hpp"
#include "../../inc/common/SymbolTable.hpp"

using namespace std;

class Linker {
public:
    Linker(const vector<string> &inputFiles, string outFile, const vector<pair<string, Elf32_Addr>> &placeDefs,
           bool hexMode);

    void run();

private:

    void getSectionSizes(unique_ptr<Elf32File> &eFile);

    void getSectionMappings();

    void generateSymbols();

    static void handleRelocations(unique_ptr<Elf32File> &eFile);

    void writeHexOutput();

    vector<unique_ptr<Elf32File>> inputFileObjects;
    string outFile;
    bool hexMode;

    vector<pair<string, Elf32_Addr>> placeDefs;

    vector<Elf32_Shdr> programSections;

    unordered_map<string, Elf32_Word> sectionSizes;

    unordered_map<string, Elf32_Addr> sectionMap;

    unordered_map<string, vector<Elf32_Sym *>> undefinedDefs;
    unordered_map<string, Elf32_Sym *> globalDefs;

    SymbolTable symbolTable;

};
