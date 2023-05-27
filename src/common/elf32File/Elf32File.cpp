#include "../../../inc/common/Elf32File.hpp"
#include "../../../inc/common/Ins32.hpp"
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