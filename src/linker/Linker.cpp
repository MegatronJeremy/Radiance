#include "../../inc/linker/Linker.hpp"

#include <utility>
#include <iomanip>

Linker::Linker(const vector<string> &inputFiles, string outFile,
               const vector<pair<string, Elf32_Addr>> &placeDefs, bool hexMode) :
        outFile(std::move(outFile)),
        placeDefs(placeDefs),
        hexMode(hexMode) {
    inputFileObjects.reserve(inputFiles.size());
    for (auto &f: inputFiles) {
        unique_ptr<Elf32File> e32 = make_unique<Elf32File>(f);
        if (e32->elfHeader.e_type != ET_REL) {
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
    if (hexMode) {
        writeHexOutput();
    }
}

void Linker::getSectionSizes(unique_ptr<Elf32File> &eFile) {
    for (Elf32_Shdr &sh: eFile->sectionHeaderTable) {
        if (hexMode && sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS) {
            // only keep program sections
            continue;
        }

        string sectionName = eFile->sectionName(sh);
        cout << "Getting size for: " << sectionName << endl;
        if (sectionSizes.find(sectionName) == sectionSizes.end()) {
            sectionSizes[sectionName] = 0;
        }
        // offset from start of section is end of last section (from zero)
        sh.sh_offset = sectionSizes[sectionName];
        // update new end of section
        sectionSizes[sectionName] += sh.sh_size;
    }
}

void Linker::getSectionMappings() {
    Elf32_Word location = 0;
    for (unique_ptr<Elf32File> &eFile: inputFileObjects) {
        for (Elf32_Shdr &sh: eFile->sectionHeaderTable) {
            if (sh.sh_type != SHT_PROGBITS && sh.sh_type != SHT_NOBITS) {
                // only keep program sections
                continue;
            }

            string sectionName = eFile->sectionName(sh);
            if (sectionMap.find(sectionName) != sectionMap.end()) { // section was already mapped, add ofsset
                sh.sh_offset += sectionMap[sectionName];
            } else {
                sh.sh_offset += location; // sh_offset starts from zero
                sectionMap[sectionName] = sh.sh_offset;
                location += sectionSizes[sectionName];
            }
        }
    }
}


void Linker::generateSymbols() {
    for (unique_ptr<Elf32File> &eFile: inputFileObjects) {
        for (Elf32_Sym &sym: eFile->symbolTable) {
            string symName = eFile->symbolName(sym);
            cout << "Handling: ---------------- " << symName << endl;

            if (sym.st_shndx == SHN_ABS || sym.st_name == SHN_UNDEF) {
                // nothing to do - symbol value is absolute or symbol is for undefined section
                continue;
            }

            if (sym.st_shndx == SHN_UNDEF) {
                if (globalDefs.find(symName) == globalDefs.end()) {
                    cout << "Adding undefined def" << endl;
                    // undefined weak symbol, add to U
                    if (undefinedDefs.find(symName) == undefinedDefs.end()) {
                        undefinedDefs[symName] = {};
                    }
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
                string sectionName = eFile->sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile->sectionHeaderTable[sym.st_shndx].sh_offset;

                cout << "three" << endl;
                // and define all undefined syms
                for (auto &it: undefinedDefs[symName]) {
                    *it = sym;
                }
                cout << "four" << endl;
                cout << "Erasing " << symName << endl;
                undefinedDefs.erase(undefinedDefs.find(symName));
                cout << "ok" << endl;
            } else {
                cout << "Update local" << endl;
                cout << sym << endl;
                // just update local symbol value
                string sectionName = eFile->sectionName(sym);
                sym.st_value += sectionMap[sectionName] + eFile->sectionHeaderTable[sym.st_shndx].sh_offset;
                cout << "ok" << endl;
            }
        }
    }
    if (!undefinedDefs.empty()) {
        string s = "Linker error: symbols without a definition: ";
        for (auto &it: undefinedDefs) {
            s += it.first + " ";
        }
        throw runtime_error(s);
    }

}

void Linker::handleRelocations(unique_ptr<Elf32File> &eFile) {
    for (auto &it: eFile->relocationTables) {
        Elf32_Section sec = it.first;
        Elf32_Sym &secSym = eFile->sectionSymbol(sec);
        vector<unsigned char> &secData = eFile->dataSections[sec - 1];
        auto *wordArray = reinterpret_cast<Elf32_Word *>(secData.data());
        cout << secSym << endl;
        for (auto &rel: it.second) {
            Elf32_Sym &sym = eFile->symbolTable[ELF32_R_SYM(rel.r_info)];
            cout << ELF32_R_SYM(rel.r_info) << " " << ELF32_R_TYPE(rel.r_info) << endl;
            cout << "Relocating: " << sym << endl;
            Elf32_Addr offsWord = rel.r_offset / sizeof(Elf32_Word);
            switch (rel.r_info) {
                case R_32S:
                case R_32:
                    // absolute relocation: S + A
                    wordArray[offsWord] = sym.st_value + rel.r_addend;
                    break;
                case R_PC32:
                    // relative relocation: S - P + A
                    wordArray[offsWord] = sym.st_value - (rel.r_offset + secSym.st_value) + rel.r_addend;
                    break;
                default:
                    continue;
            }
        }
    }
}

void Linker::writeHexOutput() {
    ofstream outF{outFile, ios::out};
    for (unique_ptr<Elf32File> &file: inputFileObjects) {
        Elf32_Section i = 1;
        for (vector<unsigned char> &data: file->dataSections) {
            cout << "Writing section: " << endl;
            cout << file->sectionName(i) << endl;
            Elf32_Word startLoc = file->sectionHeaderTable[i++].sh_offset;
            for (Elf32_Word j = 0; j < data.size(); j += 8) {
                outF << setw(4) << setfill('0') << hex << startLoc + j << ": ";

                for (Elf32_Word k = j; k < min(data.size(), static_cast<unsigned long>(j + 8)); k++) {
                    outF << setw(2) << setfill('0') << hex << static_cast<int>(data[k]) << " ";
                }

                outF << endl;
            }
        }
    }
}
