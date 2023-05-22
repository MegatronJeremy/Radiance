#include "../../inc/common/RelocationTable.hpp"

ostream &operator<<(ostream &os, const RelocationTable &rt) {
    int i = 0;
    for (const Elf32_Rela &rd: rt.relocationEntries) {
        os << rd << ": " << i++ << endl;
    }
    return os;

}

void RelocationTable::insertRelocationEntry(Elf32_Rela rd) {
    relocationEntries.emplace_back(rd);
}
