#include "../../inc/assembler/Assembler.hpp"

Elf32_Addr Assembler::getPoolConstantAddr(const PoolConstant &constant) {
    if (pass == 1) {
        return locationCounter; // do nothing
    }

    // inserts literal value in pool and returns its address
    auto &currentLiteralTable = literalTable[currentSection];
    if (currentLiteralTable.hasConstant(constant)) {
        // already has constant value, return address
        return currentLiteralTable.getConstantAddress(constant);
    }

    Elf32_Shdr &section = eFile.sectionTable.get(currentSection);

    Elf32_Addr sectionEnd = section.sh_size;
    currentLiteralTable.insertConstant(constant, sectionEnd);
    section.sh_size += WORD_LEN_BYTES;

    // sectionEnd is new literal address
    return sectionEnd;
}

void Assembler::initCurrentSectionPoolConstants() {
    auto &currentLiteralTable = literalTable[currentSection];

    while (currentLiteralTable.hasNextPoolConstant()) {
        PoolConstant nextConstant = currentLiteralTable.getNextPoolConstant();
        if (nextConstant.isNumeric) {
            initSpaceWithConstant(nextConstant.number);
        } else {
            initSpaceWithConstant(nextConstant.symbol);
        }
    }
}
