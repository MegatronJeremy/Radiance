#include "../../../inc/common/elf32file/Elf32File.hpp"

#include <iomanip>

ostream &operator<<(ostream &os, Elf32File &file) {
    os << "ELF Header:" << endl;
    os << left <<
       setw(4) << "Ident" << "\t" <<
       setw(4) << "Type" << "\t" <<
       setw(6) << "Phoff" << "\t" <<
       setw(6) << "Shoff" << "\t" <<
       setw(4) << "Phnum" << "\t" <<
       setw(4) << "Shnum" << endl;


    os << setw(4) << file.elfHeader.e_ident << "\t" <<
       setw(4) << file.elfHeader.e_type << "\t" <<
       setw(6) << file.elfHeader.e_phoff << "\t" <<
       setw(6) << file.elfHeader.e_shoff << "\t" <<
       setw(4) << file.elfHeader.e_phnum << "\t" <<
       setw(4) << file.elfHeader.e_shnum << "\t" << endl;

    os << endl;

    if (file.elfHeader.e_type == ET_REL) {
        os << "Section Headers:" << endl;
        os << setw(4) << "Num" << "\t"
           << setw(4) << "Type" << "\t"
           << setw(6) << "Offset" << "\t"
           << setw(6) << "Size" << "\t"
           << setw(4) << "Link" << "\t"
           << setw(8) << "Addr" << "\t"
           << setw(16) << "Name" << endl;

        int i = 0;
        for (Elf32_Shdr &shdr: file.sectionTable.sectionDefinitions) {
            string sectionName;
            switch (shdr.sh_type) {
                case SHT_RELA:
                    sectionName = ".rela." + file.sectionName(shdr.sh_link);
                    break;
                case SHT_STRTAB:
                    sectionName = ".strtab";
                    break;
                case SHT_SYMTAB:
                    sectionName = ".symtab";
                    break;
                default:
                    sectionName = file.symbolTable.symbolNames[shdr.sh_name];
            }
            os << setw(4) << '[' + to_string(i++) + ']' << "\t"
               << setw(4) << shdr.sh_type << "\t"
               << setw(6) << shdr.sh_offset << "\t"
               << setw(6) << shdr.sh_size << "\t"
               << setw(4) << shdr.sh_link << "\t"
               << setw(8) << right << setfill('0') << hex << shdr.sh_addr << setfill(' ') << dec << left << "\t"
               << sectionName << endl;
        }

        os << endl;

        os << "#.symtab" << endl;
        os << setw(4) << "Shndx" << "\t"
           << setw(4) << "Bind" << "\t"
           << setw(4) << "Type" << "\t"
           << setw(8) << "Value" << "\t"
           << setw(16) << "Name" << "\t" << endl;
        for (Elf32_Sym &sym: file.symbolTable.symbolDefinitions) {
            os << setw(4) << sym.st_shndx << "\t"
               << setw(4) << static_cast<Elf32_Word>(ELF32_ST_BIND(sym.st_info)) << "\t"
               << setw(4) << static_cast<Elf32_Word>(ELF32_ST_TYPE(sym.st_info)) << "\t"
               << setw(8) << right << setfill('0') << hex << sym.st_value << setfill(' ') << dec << left << "\t"
               << file.symbolTable.symbolNames[sym.st_name] << endl;
        }

        os << endl;

        for (auto &it: file.relocationTables) {
            os << "#.rela." << file.sectionName(it.first) << endl;
            RelocationTable &relaTab = it.second;
            os << setw(6) << "Offset" << "\t"
               << setw(4) << "Type" << "\t"
               << setw(4) << "Addend" << "\t"
               << setw(8) << "Symbol" << "\t" << endl;
            for (Elf32_Rela &rel: relaTab.relocationEntries) {
                os << setw(6) << rel.r_offset << "\t"
                   << setw(4) << ELF32_R_TYPE(rel.r_info) << "\t"
                   << setw(4) << rel.r_addend << "\t"
                   << file.symbolTable.symbolNames[ELF32_R_SYM(rel.r_info)] << endl;
            }
            os << endl;
        }
    } else {

        os << "Program Headers:" << endl;
        os << setw(4) << "Num" << "\t"
           << setw(6) << "Offset" << "\t"
           << setw(6) << "Memsz" << "\t"
           << setw(8) << "Addr" << endl;

        int i = 0;
        for (Elf32_Phdr &phdr: file.programTable.programDefinitions) {
            os << setw(4) << '[' + to_string(i++) + ']' << "\t"
               << setw(6) << phdr.p_offset << "\t"
               << setw(6) << phdr.p_memsz << "\t"
               << setw(8) << right << setfill('0') << hex << phdr.p_paddr << setfill(' ') << dec << left << endl;
        }
        os << endl;
    }

    for (Elf32_Word i = 1; i <= file.dataSections.size(); i++) {
        Elf32_Word startLoc = 0;
        if (file.elfHeader.e_type == ET_REL) {
            os << "#." << file.sectionName(i) << endl;

            startLoc = file.sectionTable.get(i).sh_addr;
        } else if (file.elfHeader.e_type == ET_EXEC) {
            startLoc = file.programTable.get(i).p_paddr;
        }
        stringstream &data = file.dataSections[i];

        int c;
        int j = 0;
        os << right; // set back to right-aligned for addresses
        while ((c = data.get()) != EOF) {
            if (j % 8 == 0) {
                os << setw(8) << setfill('0') << hex << startLoc + j << ": ";
            }
            if (j % 4 == 0 && j % 8 != 0) {
                os << "\t";
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

