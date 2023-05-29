#include "../../inc/assembler/Assembler.hpp"

Elf32_Sym *Assembler::insertLocalSymbol(const string &symbol) {
    if (pass == 2) {
        // do nothing
        return nullptr;
    }

    if (currentSection == SHN_UNDEF) {
        return nullptr;
    }

    Elf32_Sym *sd = eFile.symbolTable.get(symbol);

    if (sd != nullptr && sd->st_shndx == SHN_UNDEF) {
        sd->st_shndx = currentSection;
        sd->st_value = locationCounter;

        return sd;
    } else if (sd != nullptr) {
        throw runtime_error(
                "Assembler error: symbol " + eFile.symbolTable.symbolNames[sd->st_name] + " was already defined!");
    } else {
        Elf32_Sym nsd{};
        nsd.st_shndx = currentSection;
        nsd.st_value = locationCounter;
        nsd.st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE); // default values

        return eFile.symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::insertAbsoluteSymbol(const string &symbol, uint32_t symbolValue) {
    Elf32_Sym *sd = eFile.symbolTable.get(symbol);

    if (sd != nullptr && sd->st_shndx != SHN_ABS && sd->st_shndx != SHN_UNDEF) {
        throw runtime_error(
                "Assembler error: symbol " + eFile.symbolTable.symbolNames[sd->st_name] + " was already defined!");
    } else if (sd != nullptr) {
        sd->st_shndx = SHN_ABS;
        sd->st_value = symbolValue;
    } else {
        Elf32_Sym nsd{};
        nsd.st_shndx = SHN_ABS;
        nsd.st_value = symbolValue;

        eFile.symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::insertGlobalSymbol(const string &symbol) {
    if (pass == 2) {
        return; // do nothing
    }

    Elf32_Sym *sd = eFile.symbolTable.get(symbol);

    if (sd != nullptr) {
        if (ELF32_ST_TYPE(sd->st_info) == STT_SECTION) {
            cerr << "Warning: section is global by default!" << endl;
            return;
        }

        sd->st_info = ELF32_ST_INFO(STB_GLOBAL, ELF32_ST_TYPE(sd->st_info));
    } else {
        Elf32_Sym nsd;
        nsd.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE); // everything else is default

        eFile.symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::addSymbolUsage(const string &symbol) {
    if (pass == 2 || eFile.symbolTable.get(symbol) != nullptr) {
        return;
    }

    Elf32_Sym sd{}; // globalDef is false at this point

    eFile.symbolTable.insertSymbolDefinition(sd, symbol);
}

