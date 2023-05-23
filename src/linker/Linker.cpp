#include "../../inc/linker/Linker.hpp"

#include <utility>
#include <iomanip>
#include <unordered_set>
#include <map>
#include <algorithm>

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
    cout << "Getting section sizes" << endl;
    for (auto &file: inputFileObjects) {
        getSectionSizes(file);
    }
    cout << "Getting section mappings" << endl;
    getSectionMappings();
    cout << "Defining symbols" << endl;
    defineAllSymbols();
    cout << "Generating symbols " << endl;

    generateSymbols();

    cout << "Getting relocations" << endl;
    for (auto &file: inputFileObjects) {
        if (execMode) {
            handleRelocations(file);
        } else {
            generateRelocations(file);
        }
    }

    cout << "Adding sections to output" << endl;
    addSectionsToOutput();

    cout << "Writing to executable file" << endl;
    if (execMode) {
        outFile.writeExecToOutputFile(outFileName);
    } else {
        outFile.writeRelToOutputFile(outFileName);
    }
}

void Linker::getSectionSizes(Elf32File &eFile) {
    for (Elf32_Shdr &sh: eFile.sectionTable.sectionDefinitions) {
        if (sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS) {
            // only keep program sections
            continue;
        }

        string sectionName = eFile.sectionName(sh);
        cout << "Getting size for: " << sectionName << endl;
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
            bool newSection = false;
            string sectionName = eFile.sectionName(sh);
            if ((sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS)) {
                continue; // only for program sections
            }

            if (placeDefs.find(sectionName) == placeDefs.end()) { // no place definition
                cout << "No place defs for symbol " << sectionName << endl;
                if (execMode) {
                    cout << highestEnd << endl;
                    placeDefs[sectionName] = highestEnd;
                    highestEnd += sectionSizes[sectionName];
                }
            }
            if (sh.sh_addr == 0) { // start of new section
                newSection = true;
                if (execMode) { // don't check intervals for exec mode
                    intervals.push_back({placeDefs[sectionName], placeDefs[sectionName] + sectionSizes[sectionName]});
                }
            }

            sh.sh_addr += placeDefs[sectionName]; // sh_addr starts from offset inside of new section
            sh.sh_size = sectionSizes[sectionName];

            if (newSection) { // add to section table with new section index
                outFile.sectionTable.add(sh, sectionName);
            }

            cout << "Placing symbol " + sectionName + " at address " << sh.sh_addr << endl;
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


void Linker::defineAllSymbols() {
    unordered_set<string> undefinedDefs;
    unordered_set<string> globalDefs;

    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
            if (sym.st_shndx == SHN_ABS || sym.st_name == SHN_UNDEF) {
                // nothing to do - symbol value is absolute or symbol is for undefined section
                continue;
            }

            string symName = eFile.symbolName(sym);
            cout << "Handling: ---------------- " << symName << endl;

            if (sym.st_shndx == SHN_UNDEF && globalDefs.count(symName) == 0) {
                cout << "Adding undefined def" << endl;
                // undefined weak symbol, add to U
                undefinedDefs.insert(symName);

                cout << "ok" << endl;
            } else if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL) {
                // defined global symbol

                cout << "Adding global def" << endl;
                if (globalDefs.find(symName) != globalDefs.end()) {
                    throw runtime_error("Linker error: multiple definitions of symbol: " + symName);
                }

                cout << "one" << endl;
                // add sym to global defs
                globalDefs.insert(symName);

                // and erase from undefined symbols
                undefinedDefs.erase(symName);

                // subsection offset + address of new section
                string sectionName = eFile.sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;

            } else {
                cout << "Update local" << endl;
                cout << sym << endl;
                // just update local symbol value
                string sectionName = eFile.sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile.sectionTable.get(sym.st_shndx).sh_addr;
                cout << "ok" << endl;
            }
            cout << sym << endl;
        }
    }
    cout << "im here" << endl;
    if (execMode && !undefinedDefs.empty()) {
        string s = "Linker error: symbols without a definition: ";
        for (auto &it: undefinedDefs) {
            s += it + " ";
        }
        throw runtime_error(s);
    }

    cout << "done" << endl;
}

void Linker::generateSymbols() {
    for (Elf32File &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile.symbolTable.symbolDefinitions) {
            string symName = eFile.symbolName(sym);

            if (sym.st_shndx == SHN_UNDEF) {
                continue; // symbol defined in other section
            }

            Elf32_Sym nsym = sym;

            // only if section exists
            if (nsym.st_shndx != SHN_ABS && nsym.st_shndx != SHN_UNDEF) {
                string sectionName = eFile.sectionName(sym);
                cout << "Section name " << sectionName << endl;
                nsym.st_shndx = outFile.sectionTable.getSectionIndex(sectionName);
            }

            cout << "For symbol " << symName << " " << nsym << endl;

            outFile.symbolTable.insertSymbolDefinition(nsym, symName);

            if (ELF32_ST_TYPE(nsym.st_info) == STT_SECTION) {
                // section symbol, add name index to section table
                outFile.sectionTable.get(symName)->sh_name = outFile.symbolTable.getSymbolIndex(symName);
            }
        }
    }
}

void Linker::generateRelocations(Elf32File &eFile) {
    for (auto &it: eFile.relocationTables) {
        Elf32_Shdr &sh = eFile.sectionTable.get(it.first);
        string secName = eFile.sectionName(sh);
        for (Elf32_Rela &rel: it.second.relocationEntries) {
            string symName = eFile.symbolTable.symbolNames[ELF32_R_SYM(rel.r_info)];

            // symbol in out file
            Elf32_Sym *outSym = outFile.symbolTable.get(symName);

            Elf32_Rela outRela = rel;
            outRela.r_offset += sh.sh_addr; // add new section start offset (from zero)
            outRela.r_info = ELF32_R_INFO(outSym->st_name, ELF32_R_TYPE(rel.r_info));

            Elf32_Section shndx = outFile.sectionTable.getSectionIndex(secName);

            outFile.relocationTables[shndx].insertRelocationEntry(rel);
        }
    }
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

