#include "../inc/Assembler.hpp"
#include "../inc/Ins32.hpp"

#include <stdexcept>
#include <cstring>
#include <queue>

Assembler::Assembler() {
    Elf32_Sym nsd{};
    nsd.st_shndx = SHN_UNDEF;
    nsd.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);

    symbolTable.insertSymbolDefinition(nsd, "UND");

    // relocatable file
    elfHeader.e_type = ET_REL;
}

Elf32_Sym *Assembler::insertLocalSymbol(const string &symbol) {
    if (pass == 2) {
        // do nothing
        return nullptr;
    }

    if (currentSection == SHN_UNDEF) {
        return nullptr;
    }

    Elf32_Sym *sd = symbolTable.get(symbol);

    if (sd != nullptr && sd->st_shndx == SHN_UNDEF) {
        sd->st_shndx = currentSection;
        sd->st_value = locationCounter;

        return sd;
    } else if (sd != nullptr) {
        throw invalid_argument("Symbol was already defined!");
    } else {
        Elf32_Sym nsd{};
        nsd.st_shndx = currentSection;
        nsd.st_value = locationCounter;
        nsd.st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE); // default values

        return symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::insertAbsoluteSymbol(const string &symbol, uint32_t symbolValue) {
    Elf32_Sym *sd = symbolTable.get(symbol);

    if (sd != nullptr && sd->st_shndx != SHN_ABS && sd->st_shndx != SHN_UNDEF) {
        throw invalid_argument("Symbol was already defined!");
    } else if (sd != nullptr) {
        sd->st_shndx = SHN_ABS;
        sd->st_value = symbolValue;
    } else {
        Elf32_Sym nsd{};
        nsd.st_shndx = SHN_ABS;
        nsd.st_value = symbolValue;

        symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::insertGlobalSymbol(const string &symbol) {
    if (pass == 2) {
        return; // do nothing
    }

    Elf32_Sym *sd = symbolTable.get(symbol);

    if (sd != nullptr) {
        sd->st_info = ELF32_ST_INFO(STB_GLOBAL, ELF32_ST_BIND(sd->st_info));
    } else {
        Elf32_Sym nsd;
        nsd.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE); // everything else is default

        symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::addSymbolUsage(const string &symbol) {
    if (pass == 2) {
        return; // do nothing
    }

    if (currentSection == SHN_UNDEF) {
        return;
    }

    if (symbolTable.get(symbol) != nullptr) {
        // symbol was already defined
        return;
    }

    Elf32_Sym sd{}; // globalDef is false at this point

    symbolTable.insertSymbolDefinition(sd, symbol);
}

void Assembler::registerSection(const string &section) {
    if (pass == 1) {
        insertSection(section);
    } else if (pass == 2) {
        currentSection++; // next section
    }
}

void Assembler::insertSection(const string &section) {
    // open new literal table for current section
    literalTable.emplace_back();

    // insert as symbol
    currentSection++;
    Elf32_Sym *sym = insertLocalSymbol(section);
    sym->st_info = ELF32_ST_INFO(ELF32_ST_BIND(sym->st_info), STT_SECTION);

    // insert as section
    Elf32_Shdr sd{};
    sd.sh_addr = locationCounter;
    sd.sh_size = 0;

    locationCounter = 0;

    sectionTable.insertSectionDefinition(sd, section);
}

void Assembler::endAssembly() {
    sectionTable.closeLastSection(locationCounter);
}

ostream &operator<<(ostream &os, const Assembler &as) {
    return os << endl << as.sectionTable << endl << as.symbolTable << endl << as.relocationTable << endl;
}

bool Assembler::nextPass() {
    if (pass == 2)
        return false;

    if (pass == 1) {
        prepareSecondPass();
    }

    currentSection = SHN_UNDEF;
    locationCounter = 0;
    pass = pass + 1;
    return true;
}

void Assembler::insertLiteral(const string &symbol) {
    incLocationCounter();
    if (pass == 1) {
        addSymbolUsage(symbol);
    } else {
        auto &currentLiteralTable = literalTable.back();
        if (!currentLiteralTable.hasLiteral(symbol)) {
            currentLiteralTable.insertLiteral(symbol, locationCounter);
        }

        Elf32_Word literal = generateAbsoluteRelocation(symbol);
        outputFile.write((char *) &literal, sizeof(literal));
    }
}

void Assembler::insertLiteral(Elf32_Word literal) {
    incLocationCounter();
    if (pass == 1) {
        return; // do nothing
    } else {
        auto &currentLiteralTable = literalTable.back();
        if (!currentLiteralTable.hasLiteral(literal)) {
            currentLiteralTable.insertLiteral(literal, locationCounter);
        }

        outputFile.write((char *) &literal, sizeof(literal));
    }
}

void Assembler::initAscii(string ascii) {
    // remove double quotes
    ascii = ascii.substr(1, ascii.size() - 2);

    Elf32_Word size = ascii.size();
    incLocationCounter(size);

    if (pass == 1) {
        return; // do nothing
    } else {
        outputFile << ascii;
    }
}

void Assembler::incLocationCounter(Elf32_Word bytes) {
    locationCounter += bytes;
}

void Assembler::prepareSecondPass() {
    outputFile.open("a.out", ios::out | ios::binary);

    outputFile.write((char *) &elfHeader, sizeof(elfHeader));
}

Elf32_Word Assembler::generateAbsoluteRelocation(const string &symbol) {
    // for generating R_32S
    Elf32_Sym *sd = symbolTable.get(symbol);

    Elf32_Rela rd;

    rd.r_addend = 0;
    rd.r_info = ELF32_R_INFO(sd->st_name, R_32S);
    rd.r_offset = locationCounter;

    relocationTable.insertRelocationEntry(rd);

    return sd->st_value;
}


Elf32_Word Assembler::generateRelativeRelocation(const string &symbol) {
    // for generating R_PC32
    Elf32_Sym *sd = symbolTable.get(symbol);

    if (sd->st_shndx == currentSection) {
        // no need for a relocation, same section
    }

    Elf32_Rela rd;

    rd.r_addend = 0;
    rd.r_info = ELF32_R_INFO(sd->st_name, R_PC32);
    rd.r_offset = locationCounter;

    relocationTable.insertRelocationEntry(rd);

    return sd->st_value;
}

void Assembler::zeroInitSpace(Elf32_Word bytes) {
    incLocationCounter(bytes);

    char buff[bytes];
    memset(buff, 0, sizeof(buff));
    outputFile.write(buff, bytes);
}

void Assembler::insertInstruction(yytokentype token, const vector<int16_t> &fields) {
    switch (token) {
        case IRET:
            incLocationCounter(2 * INSTRUCTION_LEN_BYTES);
            break;
        default:
            incLocationCounter(INSTRUCTION_LEN_BYTES);
    }
    if (pass == 1) {
        return; // do nothing
    }

    queue<pair<yytokentype, vector<int16_t>>> buffer;
    buffer.emplace(token, fields);

    // switch for complex instruction types
    while (!buffer.empty()) {
        yytokentype nextToken = buffer.front().first;
        vector<int16_t> nextFields = buffer.front().second;
        buffer.pop();

        switch (nextToken) {
            case IRET:
                buffer.emplace(POP, vector<int16_t>{PC});
                buffer.emplace(POP_CS, vector<int16_t>{STATUS});
                break;
            default:
                Ins32 ins32 = Instruction32::getInstruction(nextToken, nextFields);
                outputFile.write((char *) &ins32, sizeof(Ins32));
        }
    }

}
