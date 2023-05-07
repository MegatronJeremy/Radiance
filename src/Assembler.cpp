#include "../inc/Assembler.hpp"

#include <stdexcept>

void Assembler::insertLocalSymbol(const string &symbol) {
    if (currentSection == UND) {
        return;
    }

    SymbolDefinition *sd = symbolTable.get(symbol);

    if (sd != nullptr && sd->section == UND) {
        sd->section = currentSection;
        sd->symbolValue = locationCounter;
    } else if (sd != nullptr) {
        throw invalid_argument("Symbol was already defined!");
    } else {
        SymbolDefinition nsd{};
        nsd.name = symbol;
        nsd.section = currentSection;
        nsd.symbolValue = locationCounter;

        symbolTable.insertSymbolDefinition(nsd);
    }
}

void Assembler::insertAbsoluteSymbol(const string &symbol, uint32_t symbolValue) {
    SymbolDefinition *sd = symbolTable.get(symbol);

    if (sd != nullptr && sd->section != ABS && sd->section != UND) {
        throw invalid_argument("Symbol was already defined!");
    } else if (sd != nullptr) {
        sd->section = ABS;
        sd->symbolValue = symbolValue;
    } else {
        SymbolDefinition nsd{};
        nsd.name = symbol;
        nsd.section = ABS;
        nsd.symbolValue = symbolValue;

        symbolTable.insertSymbolDefinition(nsd);
    }
}

void Assembler::insertGlobalSymbol(const string &symbol) {
    if (pass == 2) {
        return; // do nothing
    }

    SymbolDefinition *sd = symbolTable.get(symbol);

    if (sd != nullptr) {
        sd->globalDef = true;
    } else {
        SymbolDefinition nsd;
        nsd.globalDef = true;
        nsd.name = symbol;

        symbolTable.insertSymbolDefinition(nsd);
    }
}

void Assembler::addSymbolUsage(const string &symbol) {
    if (currentSection == UND) {
        return;
    }

    if (symbolTable.get(symbol) != nullptr) {
        // symbol was already defined
        return;
    }

    SymbolDefinition sd{}; // globalDef is false at this point
    sd.name = symbol;

    symbolTable.insertSymbolDefinition(sd);
}

void Assembler::registerSection(const string &section) {
    if (pass == 1) {
        insertSection(section);
    } else if (pass == 2) {
        currentSection++; // next section
    }
}

void Assembler::insertSection(const string &section) {
    SectionDefinition sd{};

    sd.name = section;
    sd.base = locationCounter;
    sd.length = 0;

    locationCounter = 0;

    currentSection = sectionTable.insertSectionDefinition(sd);

    insertLocalSymbol(section);
}

void Assembler::reserveSpace(const string &str) {
    locationCounter += str.size() - 2;
}

void Assembler::endAssembly() {
    sectionTable.closeLastSection(locationCounter);
}

ostream &operator<<(ostream &os, const Assembler &as) {
    return os << endl << as.sectionTable << endl << as.symbolTable << endl;
}

bool Assembler::nextPass() {
    if (pass == 2)
        return false;

    currentSection = UND;
    locationCounter = 0;
    pass = pass + 1;
    return true;
}
