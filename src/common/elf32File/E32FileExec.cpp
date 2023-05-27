#include "../../../inc/common/Elf32File.hpp"

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
