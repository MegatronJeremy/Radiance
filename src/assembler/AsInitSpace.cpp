#include "../../inc/assembler/Assembler.hpp"

#include <cstring>

void Assembler::initSpaceWithConstant(const string &symbol) {
    // to be called with .word or at the end of the second pass for the current section, to init all pool values
    if (pass == 1) {
        addSymbolUsage(symbol);
    } else {
        // generate relocation
        Elf32_Word relocationValue = generateAbsoluteRelocation(symbol);
        eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
    }
    incLocationCounter();
}

void Assembler::initSpaceWithConstant(Elf32_Word literal) {
    // to be called with .word or at the end of the second pass for the current section, to init all pool values
    if (pass == 2) {
        // generate no relocation
        eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&literal), sizeof(Elf32_Word));
    }
    incLocationCounter();
}

void Assembler::initAscii(string ascii) {
    // remove double quotes
    ascii = ascii.substr(1, ascii.size() - 2);

    Elf32_Word size = ascii.size();
    incLocationCounter(size);

    if (pass == 1) {
        return; // do nothing
    } else {
        eFile.dataSections[currentSection].write(ascii.c_str(), static_cast<streamsize>(ascii.size()));
    }
}

void Assembler::zeroInitSpace(Elf32_Word bytes) {
    incLocationCounter(bytes);

    if (pass == 1) {
        return; // do nothing
    }

    char buff[bytes];
    memset(buff, 0x0, sizeof(buff));
    eFile.dataSections[currentSection].write(buff, bytes);
}

