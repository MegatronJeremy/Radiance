#include "../../inc/common/ProgramTable.hpp"

#include <iostream>

using namespace std;


ostream &operator<<(ostream &os, const ProgramTable &pt) {
    os << "Symbol table: " << endl;
    for (const Elf32_Phdr &ph: pt.programDefinitions) {
        os << ph << endl;
    }
    return os;
}

