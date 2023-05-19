#pragma once

#include <string>
#include <fstream>
#include <utility>

#include "../../inc/common/Elf32File.hpp"

using namespace std;

class Linker {
public:
    Linker(const vector<string> &inputFiles, string outFile, const vector<pair<string, Elf32_Addr>> &placeDefs,
           bool hexMode);

private:
    vector<string> inputFiles;
    string outFile;
    vector<pair<string, Elf32_Addr>> placeDefs;
    bool hexMode;

};