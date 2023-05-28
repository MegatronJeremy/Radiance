#include "../../../inc/common/elf32file/Elf32File.hpp"

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

void Elf32File::loadSection(const Elf32_Shdr &sh, Elf32_Section &currDataSection, fstream &file) {
    file.seekg(sh.sh_offset);

    switch (sh.sh_type) {
        case SHT_PROGBITS:
        case SHT_NOBITS: {
            if (sh.sh_size > 0) { // only load if section size is not-null
                stringstream s;

                // load data section
                vector<char> v;

                v.resize(sh.sh_size);

                file.read(v.data(), sh.sh_size);

                s.write(v.data(), sh.sh_size);
                dataSections[currDataSection++] = std::move(s);
            }

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

    // write main section headers (except undefined sectiond!!!)
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

void Elf32File::writeRelDataSections(fstream &file) {
    for (size_t i = 1; i <= dataSections.size(); i++) {
        stringstream &s = dataSections[i];
        Elf32_Shdr &sh = sectionTable.get(i);
        sh.sh_offset = file.tellp(); // section 0 is reserved
        file << s.rdbuf();
    }
}

