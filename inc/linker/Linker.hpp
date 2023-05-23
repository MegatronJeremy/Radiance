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
           bool hexMode);

    void run();

private:

    void getSectionSizes(Elf32File &eFile);

    void getSectionMappings();

    void generateSymbols();

    void addSectionsToOutput();

    static void handleRelocations(Elf32File &eFile);

    vector<Elf32File> inputFileObjects;
    string outFileName;
    bool hexMode;

    Elf32File outFile;

    unordered_map<string, Elf32_Addr> placeDefs;

    vector<Elf32_Shdr> programSections;

    unordered_map<string, Elf32_Word> sectionSizes;

    unordered_map<string, Elf32_Addr> sectionMap;

    SymbolTable symbolTable;

};
