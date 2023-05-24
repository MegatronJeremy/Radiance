#include "../../inc/assembler/Assembler.hpp"
#include "../../inc/common/Util.hpp"

#include <stdexcept>
#include <cstring>
#include <queue>
#include <utility>

Elf32_Addr Assembler::locationCounter = 0;

Assembler::Assembler(string outFile) : outFile(std::move(outFile)) {
    // setting up undefined section
    eFile.addUndefinedSection();

    literalTable.emplace_back();

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

    Elf32_Sym *sd = eFile.symbolTable.get(symbol);

    if (sd != nullptr && sd->st_shndx == SHN_UNDEF) {
        sd->st_shndx = currentSection;
        sd->st_value = locationCounter;

        return sd;
    } else if (sd != nullptr) {
        throw runtime_error("Assembler error: symbol was already defined!");
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
        throw runtime_error("Assembler error: symbol was already defined!");
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
    if (pass == 2 || currentSection == SHN_UNDEF || eFile.symbolTable.get(symbol) != nullptr) {
        return;
    }

    Elf32_Sym sd{}; // globalDef is false at this point

    eFile.symbolTable.insertSymbolDefinition(sd, symbol);
}

void Assembler::registerSection(const string &section) {
    if (pass == 1) {
        insertSection(section);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        currentSection++; // next section
        locationCounter = 0; // reset location counter
    }
}

void Assembler::insertSection(const string &section) {
    // open new literal table and relocation table for current section
    literalTable.emplace_back();

    currentSection++;

    // close last section
    eFile.sectionTable.closeLastSection(locationCounter);

    // reset location counter
    locationCounter = 0;

    // insert as section
    Elf32_Shdr &sd = eFile.sectionTable.add();
    sd.sh_type = SHT_PROGBITS;

    // insert section as symbol
    Elf32_Sym *sym = insertLocalSymbol(section);

    // section always has local type (for later symbol table lookup)
    sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
    // same string table index as symbol
    sd.sh_name = sym->st_name;
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

void Assembler::endAssembly() {
    if (pass == 1) {
        eFile.sectionTable.closeLastSection(locationCounter);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        eFile.writeRelToOutputFile(outFile);
    }
}

bool Assembler::nextPass() {
    if (pass == 2)
        return false;

    currentSection = SHN_UNDEF;
    locationCounter = 0;
    pass = pass + 1;
    return true;
}

void Assembler::initSpaceWithConstant(const string &symbol) {
    // to be called with .word or at the end of the second pass for the current section, to init all pool values
    if (pass == 1) {
        addSymbolUsage(symbol);
    } else {
        // generate relocation
        generateAbsoluteRelocation(symbol);
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

void Assembler::incLocationCounter(Elf32_Word bytes) {
    locationCounter += bytes;
}


void Assembler::generateAbsoluteRelocation(const string &symbol) {
    // for generating R_32S
    Elf32_Sym *sd = eFile.symbolTable.get(symbol);
    Elf32_Rela rd;

    Elf32_Word relocationValue = 0;

    if (sd->st_shndx == SHN_ABS) {
        // already absolute, no need for relocation
        relocationValue = sd->st_value;
        return;
    } else if (sd->st_shndx != SHN_UNDEF && ELF32_ST_BIND(sd->st_info) == STB_LOCAL) {
        // if symbol is local use the section index
        rd.r_addend = static_cast<Elf32_Sword>(sd->st_value);

        Elf32_Word sectionName = eFile.sectionTable.get(sd->st_shndx).sh_name;

        rd.r_info = ELF32_R_INFO(sectionName, R_32S);
    } else {
        // if global or undefined use symbol index
        rd.r_addend = 0;
        rd.r_info = ELF32_R_INFO(sd->st_name, R_32S);
    }

    rd.r_offset = locationCounter;

    auto &currentRelaTable = eFile.relocationTables[currentSection];
    currentRelaTable.insertRelocationEntry(rd);

    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
}


void Assembler::generateRelativeRelocation(Elf32_Sym *sd) {
    // for generating R_PC32
    Elf32_Word relocationValue;
    if (sd->st_shndx == currentSection) {
        // no need for a relocation, same section
        relocationValue = sd->st_value - locationCounter - INSTRUCTION_LEN_BYTES;
    } else {
        // generate relocation value
        Elf32_Rela rd;

        if (ELF32_ST_BIND(sd->st_info) == STB_LOCAL) {
            // if symbol is local use the section index
            Elf32_Word sectionName = eFile.sectionTable.get(sd->st_shndx).sh_name;

            rd.r_addend = static_cast<Elf32_Sword>(sd->st_value - locationCounter - INSTRUCTION_LEN_BYTES);
            rd.r_info = ELF32_R_INFO(sectionName, R_PC32);
        } else {
            // use the symbol name if globally visible
            rd.r_addend = -INSTRUCTION_LEN_BYTES;
            rd.r_info = ELF32_R_INFO(sd->st_name, R_PC32);
        }

        // where to place relocation
        rd.r_offset = locationCounter;

        auto &currentRelaTable = eFile.relocationTables[currentSection];
        currentRelaTable.insertRelocationEntry(rd);

        // write zeroes
        relocationValue = 0;
    }

    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
}

void Assembler::zeroInitSpace(Elf32_Word bytes) {
    incLocationCounter(bytes);

    char buff[bytes];
    memset(buff, 0, sizeof(buff));
    eFile.dataSections[currentSection].write(buff, bytes);
}

void Assembler::insertInstruction(yytokentype token, const vector<int16_t> &fields) {
    incLocationCounter(INSTRUCTION_LEN_BYTES);

    if (pass == 1) {
        return; // do nothing
    }

    Ins32 ins32 = Instruction32::getInstruction(token, fields);
    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&ins32), sizeof(Ins32));
}

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

void Assembler::insertFlowControlIns(yytokentype type, const PoolConstant &constant,
                                     const vector<int16_t> &fields) {
    if (pass == 1 && !constant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(constant.symbol);
    }

    if (constant.isNumeric && positiveValueFitsInDisp(constant.number)) {
        insertInstruction(type, {R0, fields[REG_B], fields[REG_C], static_cast<int16_t>(constant.number)});
    } else {
        Elf32_Addr poolConstantAddr = getPoolConstantAddr(constant);
        int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr, locationCounter);

        // TODO an additional check for free registers?
        insertInstruction(PUSH, {R13});
        insertInstruction(LD_PCREL, {R13, R0, offsetToPoolLiteral});
        insertInstruction(type, {R13, fields[REG_B], fields[REG_C]});
        insertInstruction(POP, {R13});
    }
}

void Assembler::insertLoadIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (pass == 1 && !poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    // will always pass all its register fields
    if (poolConstant.isNumeric && positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
    } else {
        Elf32_Addr poolConstantAddr = getPoolConstantAddr(PoolConstant{poolConstant.symbol});
        int16_t offsetToPoolLiteral = getDisplacement(locationCounter, poolConstantAddr);

        insertInstruction(LD_PCREL, {fields[REG_A], R0, offsetToPoolLiteral});

        // Use REG_A as additional accumulator in place of literal value (already part of fields),
        // except for LD_IMM, where values is already loaded
        if (type != LD_REG) {
            insertInstruction(type, fields);
        }
    }
}

void Assembler::insertStoreIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (pass == 1 && !poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    if (poolConstant.isNumeric && positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
    } else {
        Elf32_Addr poolConstantAddr = getPoolConstantAddr(PoolConstant{poolConstant.symbol});
        int16_t offsetToPoolLiteral = getDisplacement(locationCounter, poolConstantAddr);

        // TODO an additional check for free registers?
        insertInstruction(PUSH, {R13});
        insertInstruction(LD_PCREL, {R13, R0, offsetToPoolLiteral});
        insertInstruction(type, {R13, fields[REG_B], fields[REG_C]}); // REG_A field is always free
        insertInstruction(POP, {R13});
    }
}

void Assembler::insertIretIns() {
    insertInstruction(POP, {PC});
    insertInstruction(POP, {STATUS});
}

