#include "../inc/RelocationTable.hpp"

ostream &operator<<(ostream &os, const Elf32_Rela &rd) {
    return os << rd.r_offset << " " << ELF32_R_SYM(rd.r_info) << " " << ELF32_R_TYPE(rd.r_info) << " " << rd.r_addend;
}

ostream &operator<<(ostream &os, const RelocationTable &rt) {
    os << "Relocation table: " << endl;
    int i = 0;
    for (const Elf32_Rela &rd: rt.relocationEntries) {
        os << rd << ": " << i++ << endl;
    }
    return os;

}

void RelocationTable::insertRelocationEntry(Elf32_Rela rd) {
    relocationEntries.emplace_back(rd);
}
