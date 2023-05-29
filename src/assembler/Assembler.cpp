#include "../../inc/assembler/Assembler.hpp"

#include <stdexcept>
#include <utility>

Elf32_Addr Assembler::locationCounter = 0;

Assembler::Assembler(string outFile) : outFile(std::move(outFile)) {
    // setting up undefined section
    eFile.addUndefinedSection();

    literalTable.emplace_back();

    // relocatable file
    elfHeader.e_type = ET_REL;
}

bool Assembler::nextPass() {
    if (pass == 2)
        return false;

    // resolve TNS before second pass
    resolveTNS();

    // reset location counter here
    currentSection = SHN_UNDEF;
    locationCounter = 0;

    pass = pass + 1;

    return true;
}

void Assembler::endAssembly() {
    if (pass == 1) {
        eFile.sectionTable.closeLastSection(locationCounter);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        eFile.writeRelToOutputFile(outFile);
    }
}

