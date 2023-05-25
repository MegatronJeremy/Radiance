#include "../../inc/common/Elf32File.hpp"
#include "../../inc/common/Ins32.hpp"
#include <cstring>
#include <iomanip>

void Elf32File::addUndefinedSection() {
    Elf32_Sym nsd{};
    nsd.st_shndx = SHN_UNDEF;
    nsd.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);

    symbolTable.insertSymbolDefinition(nsd, "UND");
    Elf32_Shdr nsh{};

    sectionTable.add(nsh, "UND");
}

void Elf32File::loadFromInputFile(const string &fileName) {
    // load file and everything that goes with it
    fstream file{fileName, ios::in | ios::out | ios::binary};

    file.read(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));

    if (strcmp(reinterpret_cast<const char *>(elfHeader.e_ident), ELFMAG) != 0) {
        throw runtime_error("Readobj error: invalid ELF format!");
    }

    if (elfHeader.e_type == ET_EXEC) {
        loadExecFile(file);
    } else if (elfHeader.e_type == ET_REL) {
        loadRelFile(file);
    } else {
        throw runtime_error("Readobj error: unrecognized ELF file type");
    }
}

void Elf32File::loadRelFile(fstream &file) {
    // seek to section header offset from file start
    file.seekg(elfHeader.e_shoff, ios::beg);

    // load section headers
    for (auto i = 0; i < elfHeader.e_shnum; i++) {
        Elf32_Shdr sh;
        file.read(reinterpret_cast<char *>(&sh), sizeof(Elf32_Shdr));

        sectionTable.add(sh);
    }

    // read sections from section headers
    Elf32_Section currDataSection = 1;
    for (auto &sh: sectionTable.sectionDefinitions) {
        loadSection(sh, currDataSection, file);
    }
}

void Elf32File::loadExecFile(fstream &file) {
    // seek to program header offset from file start
    file.seekg(elfHeader.e_phoff, ios::beg);

    // load program section headers
    for (auto i = 0; i < elfHeader.e_phnum; i++) {
        Elf32_Phdr ph;
        file.read(reinterpret_cast<char *>(&ph), sizeof(Elf32_Phdr));

        programTable.add(ph);
    }

    // read sections from section headers
    Elf32_Section currDataSection = 1;
    for (auto &ph: programTable.programDefinitions) {
        loadSection(ph, currDataSection, file);
    }
}

void Elf32File::loadSection(const Elf32_Phdr &ph, Elf32_Section &currDataSection, fstream &file) {
    file.seekg(ph.p_offset);
    // load data section
    vector<char> v;

    v.resize(ph.p_memsz);

    file.read(v.data(), ph.p_memsz);

    stringstream s;

    s.write(v.data(), ph.p_memsz);

    dataSections[currDataSection++] = std::move(s);
}

void Elf32File::loadSection(const Elf32_Shdr &sh, Elf32_Section &currDataSection, fstream &file) {
    file.seekg(sh.sh_offset);

    switch (sh.sh_type) {
        case SHT_PROGBITS:
        case SHT_NOBITS: {
            // load data section
            vector<char> v;

            v.resize(sh.sh_size);

            file.read(v.data(), sh.sh_size);

            stringstream s;

            s.write(v.data(), sh.sh_size);

            dataSections[currDataSection++] = std::move(s);
            break;
        }
        case SHT_STRTAB: {
            // string table
            Elf32_Word sz = sh.sh_size;

            Elf32_Word currSym = 0;
            while (sz > 0) {
                string s;
                std::getline(file, s, '\0');
                symbolTable.symbolNames.push_back(s);
                symbolTable.symbolMappings[s] = currSym++;

                sz -= s.size() + 1; // +1 for \0
            }
            break;
        }
        case SHT_SYMTAB: {
            // symbol table
            Elf32_Word sz = sh.sh_size;

            while (sz > 0) {
                Elf32_Sym sd;
                file.read(reinterpret_cast<char *>(&sd), sizeof(Elf32_Sym));
                symbolTable.symbolDefinitions.push_back(sd);

                sz -= sizeof(Elf32_Sym);
            }
            break;
        }
        case SHT_RELA: {
            // relocation table entries for section

            RelocationTable r;

            Elf32_Word sz = sh.sh_size;

            while (sz > 0) {
                Elf32_Rela rd;
                file.read(reinterpret_cast<char *>(&rd), sizeof(Elf32_Rela));
                r.insertRelocationEntry(rd);

                sz -= sizeof(Elf32_Rela);
            }

            relocationTables[sh.sh_link] = r;
            break;
        }
        default:
            break;
    }
}

ostream &operator<<(ostream &os, Elf32File &file) {
    os << "Header: " << endl;
    os << file.elfHeader << endl << endl;

    if (file.elfHeader.e_type == ET_REL) {
        os << file.sectionTable << endl;

        os << file.symbolTable << endl;

        os << "String table: " << endl;
        for (Elf32_Word i = 0; i < file.symbolTable.symbolNames.size(); i++) {
            os << i << ": " << file.symbolTable.symbolNames[i] << endl;
        }
        os << endl;

        os << "Relocation tables: " << endl;
        for (auto &it: file.relocationTables) {
            os << file.sectionName(it.first) << ": " << endl;
            os << it.second << endl;
        }
    }

    for (Elf32_Word i = 1; i <= file.dataSections.size(); i++) {
        Elf32_Word startLoc = 0;
        if (file.elfHeader.e_type == ET_REL) {
            os << "Contents of section: " << file.sectionName(i) << endl;
            startLoc = file.sectionTable.get(i).sh_addr;
        } else if (file.elfHeader.e_type == ET_EXEC) {
            startLoc = file.programTable.get(i).p_paddr;
        }
        stringstream &data = file.dataSections[i];

        int c;
        int j = 0;
        while ((c = data.get()) != EOF) {
            if (j % 8 == 0) {
                os << setw(4) << setfill('0') << hex << startLoc + j << ": ";
            }
            j++;
            os << setw(2) << setfill('0') << hex << c;
            if (j % 8 == 0) {
                os << endl;
            } else {
                os << " ";
            }
        }
        if (j % 8 != 0) {
            os << endl;
        }
        os << endl;
    }

    return os;
}

void Elf32File::writeSymbolTable(vector<Elf32_Shdr> &additionalHeaders, fstream &file) {
    // add symbol table section header
    Elf32_Shdr sh{};
    sh.sh_size = symbolTable.symbolDefinitions.size() * sizeof(Elf32_Sym);
    sh.sh_type = SHT_SYMTAB;
    sh.sh_link = SHT_STRTAB;
    // current position of file
    sh.sh_offset = file.tellp();

    additionalHeaders.push_back(sh);

    // symtab
    for (auto &sd: symbolTable.symbolDefinitions) {
        file.write(reinterpret_cast<const char *>(&sd), sizeof(sd));
    }
}

void Elf32File::writeStringTable(vector<Elf32_Shdr> &additionalHeaders, fstream &file) {
    // add symbol table section header
    Elf32_Shdr sh{};
    sh.sh_type = SHT_STRTAB;
    // current position of file
    sh.sh_offset = file.tellp();


    // strtab
    for (auto &str: symbolTable.symbolNames) {
        const char *cst = str.c_str();
        file.write(cst, static_cast<long>(strlen(cst) + 1)); // +1 for terminator
        sh.sh_size += strlen(cst) + 1;
    }

    additionalHeaders.push_back(sh);
}

void Elf32File::writeRelocationTables(vector<Elf32_Shdr> &additionalHeaders, fstream &file) {
    // rela tables
    for (Elf32_Word sec = 1; sec <= dataSections.size(); sec++) {
        auto &relaTable = relocationTables[sec];

        // if no relocation entries skip
        if (relaTable.relocationEntries.empty()) {
            continue;
        }

        Elf32_Shdr sh{};
        sh.sh_size = relaTable.relocationEntries.size() * sizeof(Elf32_Rela);
        sh.sh_link = sec;
        sh.sh_type = SHT_RELA;
        sh.sh_offset = file.tellp();

        // add relocation table section header
        additionalHeaders.push_back(sh);

        for (auto &rd: relaTable.relocationEntries) {
            file.write(reinterpret_cast<char *>(&rd), sizeof(rd));
        }
    }
}

void Elf32File::writeRelDataSections(fstream &file) {
    for (size_t i = 1; i <= dataSections.size(); i++) {
        stringstream &s = dataSections[i];
        Elf32_Shdr &sh = sectionTable.get(i);
        sh.sh_offset = file.tellp(); // section 0 is reserved
        file << s.rdbuf();
    }
}

void Elf32File::writeExecDataSections(fstream &file) {
    for (size_t i = 1; i <= dataSections.size(); i++) {
        stringstream &s = dataSections[i];

        Elf32_Shdr &sh = sectionTable.get(i);

        Elf32_Phdr ph;
        ph.p_memsz = sh.sh_size;
        ph.p_offset = file.tellp();
        ph.p_paddr = sh.sh_addr;

        programTable.add(ph);

        file << s.rdbuf();
    }
}

void Elf32File::writeRelToOutputFile(const string &fileName) {
    fstream file{fileName, ios::out | ios::binary};

    elfHeader.e_type = ET_REL;

    // first the header
    file.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));

    // what is written here: Elf Header, Sections
    // Need to do: symtab, strtab, rela

    vector<Elf32_Shdr> additionalHeaders;

    // write these first, before section header start
    writeRelDataSections(file);

    writeRelocationTables(additionalHeaders, file);

    writeSymbolTable(additionalHeaders, file);

    writeStringTable(additionalHeaders, file);

    elfHeader.e_shoff = file.tellp(); // section header offset

    // all written section headers
    elfHeader.e_shnum = sectionTable.sectionDefinitions.size() + additionalHeaders.size();

    // write main section headers (except undefined sectiond!!!) TODO - check this everywhere else
    for (Elf32_Shdr &sh: sectionTable.sectionDefinitions) {
        file.write(reinterpret_cast<char *>(&sh), sizeof(sh));
    }

    // write additional section headers
    for (auto &sh: additionalHeaders) {
        file.write(reinterpret_cast<char *>(&sh), sizeof(sh));
    }

    // write section header again to start of file, now with correct information
    file.seekp(SEEK_SET);
    file.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));
}

void Elf32File::writeExecToOutputFile(const string &fileName) {
    fstream file{fileName, ios::out | ios::binary};

    elfHeader.e_type = ET_EXEC;
    elfHeader.e_phnum = dataSections.size();

    // first the header
    file.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));

    // write the program sections
    writeExecDataSections(file);

    // offset to program header
    elfHeader.e_phoff = file.tellp();

    // write program section headers
    for (auto &ph: programTable.programDefinitions) {
        file.write(reinterpret_cast<char *>(&ph), sizeof(Elf32_Phdr));
    }

    file.seekg(SEEK_SET);
    file.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));
}



