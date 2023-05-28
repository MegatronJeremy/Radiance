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

    string type;
    switch (file.elfHeader.e_type) {
        case ET_REL:
            type = "REL";
            break;
        case ET_EXEC:
            type = "EXEC";
            break;
        default:
            type = "NONE";
    }

    os << setw(4) << file.elfHeader.e_ident << "\t" <<
       setw(4) << type << "\t" <<
       setw(6) << file.elfHeader.e_phoff << "\t" <<
       setw(6) << file.elfHeader.e_shoff << "\t" <<
       setw(4) << file.elfHeader.e_phnum << "\t" <<
       setw(4) << file.elfHeader.e_shnum << "\t" << endl;

    os << endl;

    if (file.elfHeader.e_type == ET_REL) {
        os << "Section Headers:" << endl;
        os << setw(4) << "Num" << "\t"
           << setw(6) << "Type" << "\t"
           << setw(6) << "Offset" << "\t"
           << setw(6) << "Size" << "\t"
           << setw(4) << "Link" << "\t"
           << setw(8) << "Addr" << "\t"
           << setw(16) << "Name" << endl;

        int i = 0;
        for (Elf32_Shdr &shdr: file.sectionTable.sectionDefinitions) {
            string sectionName;
            string sectionType;
            switch (shdr.sh_type) {
                case SHT_RELA:
                    type = "RELA";
                    sectionName = ".rela." + file.sectionName(shdr.sh_link);
                    break;
                case SHT_STRTAB:
                    type = "STRT";
                    sectionName = ".strtab";
                    break;
                case SHT_SYMTAB:
                    type = "SYMT";
                    sectionName = ".symtab";
                    break;
                case SHT_NOBITS:
                    type = "NOBIT";
                    sectionName = file.symbolTable.symbolNames[shdr.sh_name];
                    break;
                case SHT_PROGBITS:
                    type = "PROGB";
                    sectionName = file.symbolTable.symbolNames[shdr.sh_name];
                    break;
                default:
                    type = "NULL";
                    sectionName = file.symbolTable.symbolNames[shdr.sh_name];
                    break;

            }
            os << setw(4) << '[' + to_string(i++) + ']' << "\t"
               << setw(6) << type << "\t"
               << setw(6) << shdr.sh_offset << "\t"
               << setw(6) << shdr.sh_size << "\t"
               << setw(4) << shdr.sh_link << "\t"
               << setw(8) << right << setfill('0') << hex << shdr.sh_addr << setfill(' ') << dec << left << "\t"
               << sectionName << endl;
        }

        os << endl;

        os << "#.symtab" << endl;
        os << setw(4) << "Shndx" << "\t"
           << setw(6) << "Bind" << "\t"
           << setw(6) << "Type" << "\t"
           << setw(8) << "Value" << "\t"
           << setw(16) << "Name" << "\t" << endl;
        for (Elf32_Sym &sym: file.symbolTable.symbolDefinitions) {
            string bind;
            string shndx;
            if (ELF32_ST_BIND(sym.st_info) == STB_GLOBAL) {
                bind = "GLOB";
            } else {
                bind = "LOC";
            }

            if (ELF32_ST_TYPE(sym.st_info) == STT_SECTION) {
                type = "SECT";
            } else {
                type = "NOTYP";
            }

            switch (sym.st_shndx) {
                case SHN_UNDEF:
                    shndx = "*UND*";
                    break;
                case SHN_ABS:
                    shndx = "*ABS*";
                    break;
                default:
                    shndx = to_string(sym.st_shndx);
            }

            os << setw(4) << shndx << "\t"
               << setw(6) << bind << "\t"
               << setw(6) << type << "\t"
               << setw(8) << right << setfill('0') << hex << sym.st_value << setfill(' ') << dec << left << "\t"
               << file.symbolTable.symbolNames[sym.st_name] << endl;
        }

        os << endl;

        for (auto &it: file.relocationTables) {
            os << "#.rela." << file.sectionName(it.first) << endl;
            RelocationTable &relaTab = it.second;
            os << setw(6) << "Offset" << "\t"
               << setw(6) << "Type" << "\t"
               << setw(4) << "Addend" << "\t"
               << setw(8) << "Symbol" << "\t" << endl;

            for (Elf32_Rela &rel: relaTab.relocationEntries) {
                switch (ELF32_R_TYPE(rel.r_info)) {
                    case R_32:
                        type = "R_32";
                        break;
                    case R_PC32:
                        type = "R_PC32";
                        break;
                    default:
                        type = "NONE";
                }

                os << setw(6) << rel.r_offset << "\t"
                   << setw(6) << type << "\t"
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

