#include "../../inc/common/Elf32File.hpp"
#include "../../inc/common/Ins32.hpp"
#include <cstring>

Elf32File::Elf32File(const string &fileName) {
    // load file and everything that goes with it
    file.open(fileName, ios::in | ios::binary);

    file.read(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));

    if (strcmp(reinterpret_cast<const char *>(elfHeader.e_ident), ELFMAG) != 0) {
        throw runtime_error("Readobj error: invalid ELF format!");
    }

    // seek to section header offset from file start
    file.seekg(elfHeader.e_shoff, ios::beg);

    // load section headers
    for (auto i = 0; i < elfHeader.e_shnum; i++) {
        Elf32_Shdr sh;
        file.read(reinterpret_cast<char *>(&sh), sizeof(Elf32_Shdr));

        sectionHeaderTable.emplace_back(sh);
    }

    // read sections from section headers
    for (auto &sh: sectionHeaderTable) {
        loadSection(sh);
    }
}

void Elf32File::loadSection(const Elf32_Shdr &sh) {
    file.seekg(sh.sh_offset);

    switch (sh.sh_type) {
        case SHT_PROGBITS:
        case SHT_NOBITS: {
            // load data section
            vector<unsigned char> v;

            v.resize(sh.sh_size);

            file.read(reinterpret_cast<char *>(v.data()), sh.sh_size);

            dataSections.emplace_back(v);
            break;
        }
        case SHT_STRTAB: {
            // string table
            Elf32_Word sz = sh.sh_size;

            while (sz > 0) {
                string s;
                getline(file, s, '\0');
                stringTable.emplace_back(s);

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
                symbolTable.push_back(sd);

                sz -= sizeof(Elf32_Sym);
            }
            break;
        }
        case SHT_RELA: {
            // relocation table entries for section

            vector<Elf32_Rela> v;

            Elf32_Word sz = sh.sh_size;

            while (sz > 0) {
                Elf32_Rela rd;
                file.read(reinterpret_cast<char *>(&rd), sizeof(Elf32_Rela));
                v.emplace_back(rd);

                sz -= sizeof(Elf32_Rela);
            }

            relocationTables[sh.sh_link] = v;
            break;
        }
        default:
            break;
    }
}

ostream &operator<<(ostream &os, const Elf32File &file) {
    os << "Header: " << endl;
    os << file.elfHeader << endl;
    os << "Relocation tables: " << endl;
    for (auto &it: file.relocationTables) {
        os << file.stringTable[file.sectionHeaderTable[it.first].sh_name] << ": " << endl;
        for (auto &rd: it.second) {
            os << rd << endl;
        }
    }
    os << "Symbol table: " << endl;
    for (auto &sd: file.symbolTable) {
        os << sd << endl;
    }
    os << "String table: " << endl;
    for (Elf32_Word i = 0; i < file.stringTable.size(); i++) {
        os << i << ": " << file.stringTable[i] << endl;
    }
    os << "Section headers: " << endl;
    for (auto &sh: file.sectionHeaderTable) {
        os << sh << endl;
    }
    for (Elf32_Word i = 0; i < file.dataSections.size(); i++) {
        cout << "Contents of section: " << file.sectionName(i + 1) << endl;
        for (auto &word: file.dataSections[i]) {
            cout << hex << word << endl;
        }
    }
    return os;
}

Elf32File::~Elf32File() {
    file.close();
}


