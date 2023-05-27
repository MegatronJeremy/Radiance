#include "../../inc/linker/Linker.hpp"

#include <algorithm>

void Linker::getSectionSizes(Elf32File &eFile) {
    for (Elf32_Shdr &sh: eFile.sectionTable.sectionDefinitions) {
        if (sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS) {
            // only keep program sections
            continue;
        }

        string sectionName = eFile.sectionName(sh);
        sh.sh_addr = sectionSizes[sectionName];
        // update new end of section
        sectionSizes[sectionName] += sh.sh_size;
    }
}

void Linker::getSectionMappings() {
    Elf32_Addr highestEnd = 0;

    for (auto &it: placeDefs) {
        highestEnd = max(highestEnd, it.second + sectionSizes[it.first]);
    }

    vector<vector<Elf32_Addr>> intervals;

    // first all place definitions
    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Shdr &sh: eFile.sectionTable.sectionDefinitions) {
            string sectionName = eFile.sectionName(sh);
            if ((sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS)) {
                continue; // only for program sections
            }

            if (placeDefs.find(sectionName) == placeDefs.end() && execMode) { // no place definition
                placeDefs[sectionName] = highestEnd;
                highestEnd += sectionSizes[sectionName];
            }

            bool newSection = sh.sh_addr == 0; // start of new section

            sh.sh_addr += placeDefs[sectionName]; // sh_addr starts from offset inside of new section
            sh.sh_size = sectionSizes[sectionName];

            if (newSection) { // add to section table with new section index
                outFile.sectionTable.add(sh, sectionName);
                if (execMode) { // check interval overlap in exec mode
                    intervals.push_back({placeDefs[sectionName], placeDefs[sectionName] + sectionSizes[sectionName]});
                }
            }
        }
    }

    // check for overlapping intervals
    size_t n = intervals.size();
    if (n == 0) { // no sections
        return;
    }

    sort(intervals.begin(), intervals.end()); // sort by section start
    for (size_t i = 1; i < n; i++) {
        if (intervals[i][0] < intervals[i - 1][1]) { // start of next section comes before end of previous section
            // overlapping
            throw runtime_error("Linker error: overlapping data sections");
        }
    }
}

void Linker::addSectionsToOutput() {
    for (auto &file: inputFileObjects) {
        for (size_t i = 1; i <= file.dataSections.size(); i++) {
            string sectionName = file.sectionName(i);

            Elf32_Section outSection = outFile.sectionTable.getSectionIndex(sectionName);

            if (outFile.dataSections.find(outSection) == outFile.dataSections.end()) {
                // new section
                outFile.dataSections[outSection] = std::move(file.dataSections[i]);
            } else {
                // append to last section
                outFile.dataSections[outSection] << file.dataSections[i].rdbuf();
            }
        }
    }
}
