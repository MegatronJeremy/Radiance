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
    Linker(const vector<string> &inputFiles, string outFile, const unordered_map<string, Elf32_Addr> &placeDefs,
           bool execMode);

    void run();

private:

    void getSectionSizes(Elf32File &eFile);

    void getSectionMappings();

    void defineAllSymbols();

    void generateSymbols();

    void addSectionsToOutput();

    void handleRelocations(Elf32File &eFile);

    void generateRelocations(Elf32File &eFile);

    vector<Elf32File> inputFileObjects;
    string outFileName;

    bool execMode;

    Elf32File outFile;

    unordered_map<string, Elf32_Addr> placeDefs;

    vector<Elf32_Shdr> programSections;

    unordered_map<string, Elf32_Word> sectionSizes;

    unordered_map<string, Elf32_Addr> sectionMap;

    SymbolTable symbolTable;
};
