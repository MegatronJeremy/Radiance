#include "../../inc/linker/Linker.hpp"

#include <utility>
#include <map>

Linker::Linker(const vector<string> &inputFiles, string outFileName,
               const unordered_map<string, Elf32_Addr> &placeDefs, bool execMode) :
        outFileName(std::move(outFileName)),
        execMode(execMode),
        placeDefs(placeDefs) {
    for (auto &f: inputFiles) {
        Elf32File e32;
        e32.loadFromInputFile(f);

        if (e32.elfHeader.e_type != ET_REL) {
            throw runtime_error("Linker error: non-relocatable file " + f + " named as input");
        }
        inputFileObjects.emplace_back(std::move(e32));
    }

    // setting up out file
    outFile.addUndefinedSection();

    if (!execMode) {
        // ignore place defs
        this->placeDefs.clear();
    }
}

void Linker::run() {
    for (auto &file: inputFileObjects) {
        getSectionSizes(file);
    }
    getSectionMappings();
    defineAllSymbols();

    generateSymbols();

    for (auto &file: inputFileObjects) {
        if (execMode) {
            handleRelocations(file);
        } else {
            generateRelocations(file);
        }
    }

    addSectionsToOutput();

    if (execMode) {
        outFile.writeExecToOutputFile(outFileName);
    } else {
        outFile.writeRelToOutputFile(outFileName);
    }
}