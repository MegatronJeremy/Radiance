#include "../../inc/linker/Linker.hpp"

#include <utility>
#include <iomanip>
#include <unordered_set>
#include <map>
#include <algorithm>

Linker::Linker(const vector<string> &inputFiles, string outFile,
               const unordered_map<string, Elf32_Addr> &placeDefs, bool hexMode) :
        outFileName(std::move(outFile)),
        hexMode(hexMode),
        placeDefs(placeDefs) {
    for (auto &f: inputFiles) {
        Elf32File e32;
        e32.loadFromInputFile(f);

        if (e32.elfHeader.e_type != ET_REL) {
            throw runtime_error("Linker error: non-relocatable file " + f + " named as input");
        }
        inputFileObjects.emplace_back(std::move(e32));
    }
}

void Linker::run() {
    cout << "Getting section sizes" << endl;
    for (auto &file: inputFileObjects) {
        getSectionSizes(file);
    }
    cout << "Getting section mappings" << endl;
    getSectionMappings();
    cout << "Getting symbols" << endl;
    generateSymbols();
    cout << "Getting relocations" << endl;
    for (auto &file: inputFileObjects) {
        handleRelocations(file);
    }

    cout << "Adding sections to output" << endl;
    addSectionsToOutput();

    cout << "Writing to executable file" << endl;
    if (hexMode) {
        outFile.writeExecToOutputFile(outFileName);
    }
}

void Linker::getSectionSizes(Elf32File &eFile) {
    for (Elf32_Shdr &sh: eFile.sectionTable.sectionDefinitions) {
        if (hexMode && sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS) {
            // only keep program sections
            continue;
        }

        string sectionName = eFile.sectionName(sh);
        cout << "Getting size for: " << sectionName << endl;
        if (sectionSizes.find(sectionName) == sectionSizes.end()) {
            sectionSizes[sectionName] = 0;
        }
        // offset from start of section is end of last section (from zero)
        sh.sh_addr = sectionSizes[sectionName];
        // update new end of section
        sectionSizes[sectionName] += sh.sh_size;
    }
}

void Linker::getSectionMappings() {
    Elf32_Addr highestEnd = 0;

    vector<vector<Elf32_Addr>> intervals;

    // first all place definitions
    for (Elf32File &eFile: inputFileObjects) {

        for (Elf32_Shdr &sh: eFile.sectionTable.sectionDefinitions) {
            string sectionName = eFile.sectionName(sh);
            if ((sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS)) {
                continue; // only for program sections
            }

            if (placeDefs.find(sectionName) == placeDefs.end()) { // no place definition
                placeDefs[sectionName] = highestEnd;
            }
            if (sh.sh_addr == 0) { // start of section - add interval
                intervals.emplace_back(placeDefs[sectionName], placeDefs[sectionName] + sectionSizes[sectionName]);
            }

            sh.sh_addr += placeDefs[sectionName]; // sh_addr starts from offset inside of new section

            highestEnd = max(highestEnd, placeDefs[sectionName] + sectionSizes[sectionName]);
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


void Linker::generateSymbols() {
    unordered_map<string, vector<Elf32_Sym *>> undefinedDefs;
    unordered_map<string, Elf32_Sym *> globalDefs;

    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
            if (sym.st_shndx == SHN_ABS || sym.st_name == SHN_UNDEF) {
                // nothing to do - symbol value is absolute or symbol is for undefined section
                continue;
            }

            string symName = eFile.symbolName(sym);
            cout << "Handling: ---------------- " << symName << endl;

            if (sym.st_shndx == SHN_UNDEF) {
                if (globalDefs.find(symName) == globalDefs.end()) {
                    cout << "Adding undefined def" << endl;
                    // undefined weak symbol, add to U
                    undefinedDefs[symName].emplace_back(&sym);

                    cout << "ok" << endl;
                } else {
                    // add symbol definition
                    cout << "Adding sym def" << endl;
                    sym = *globalDefs[symName];
                    cout << "OK" << endl;
                }
            } else if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL) {
                // defined global symbol

                cout << "Adding global def" << endl;
                if (globalDefs.find(symName) != globalDefs.end()) {
                    throw runtime_error("Linker error: multiple definitions of symbol: " + symName);
                }

                cout << "one" << endl;
                // add sym to global defs
                globalDefs[symName] = &sym;

                cout << "two" << endl;
                // subsection offset + address of new section
                string sectionName = eFile.sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;

                cout << "three" << endl;
                // and define all undefined syms
                for (auto &it: undefinedDefs[symName]) {
                    *it = sym;
                }
                cout << "four" << endl;
                cout << "Erasing " << symName << endl;
                undefinedDefs.erase(symName);

                cout << "ok" << endl;
            } else {
                cout << "Update local" << endl;
                cout << sym << endl;
                // just update local symbol value
                string sectionName = eFile.sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;
                cout << "ok" << endl;
            }
        }
    }
    cout << "im here" << endl;
    if (!undefinedDefs.empty()) {
        string s = "Linker error: symbols without a definition: ";
        for (auto &it: undefinedDefs) {
            s += it.first + " ";
        }
        throw runtime_error(s);
    }

    cout << "done" << endl;
}

void Linker::handleRelocations(Elf32File &eFile) {
    for (auto &it: eFile.relocationTables) {
        Elf32_Section sec = it.first;
        Elf32_Sym &secSym = eFile.sectionSymbol(sec);
        stringstream &secData = eFile.dataSections[sec];
        cout << secSym << endl;
        for (auto &rel: it.second.relocationEntries) {
            Elf32_Sym &sym = eFile.symbolTable.symbolDefinitions[ELF32_R_SYM(rel.r_info)];
            cout << ELF32_R_SYM(rel.r_info) << " " << ELF32_R_TYPE(rel.r_info) << endl;
            cout << "Relocating: " << sym << endl;

            Elf32_Addr relVal;

            switch (ELF32_R_TYPE(rel.r_info)) {
                case R_32S:
                case R_32:
                    // absolute relocation: S + A
                    relVal = sym.st_value + rel.r_addend;
                    secData.seekp(rel.r_offset);
                    break;
                case R_PC32:
                    // relative relocation: S - P + A
                    relVal = sym.st_value - (rel.r_offset + secSym.st_value) + rel.r_addend;
                    secData.seekp(rel.r_offset);
                    secData.write(reinterpret_cast<char *>(&sym.st_value), sizeof(Elf32_Addr));
                    break;
                default:
                    throw runtime_error("Linker error: unrecognized relocation type");
            }

            secData.write(reinterpret_cast<char *>(&relVal), sizeof(Elf32_Addr));
        }
    }
}

void Linker::addSectionsToOutput() {
    outFile.sectionTable.add({}); // und section
    size_t lastSection = 1;
    for (auto &file: inputFileObjects) {
        for (size_t i = 1; i <= file.dataSections.size(); i++) {
            Elf32_Shdr sh = file.sectionTable.get(i);
            outFile.sectionTable.add(sh);
            outFile.dataSections[lastSection++] = std::move(file.dataSections[i]);
        }
    }
}

