#include "../inc/Assembler.hpp"
#include "../inc/Util.hpp"

#include <stdexcept>
#include <cstring>
#include <queue>
#include <utility>

Elf32_Addr Assembler::locationCounter = 0;

Assembler::Assembler(string outFile) : outFile(std::move(outFile)) {
    // setting up undefined section
    Elf32_Sym nsd{};
    nsd.st_shndx = SHN_UNDEF;
    nsd.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);

    symbolTable.insertSymbolDefinition(nsd, "UND");
    Elf32_Shdr nsh{};

    sectionTable.insertSectionDefinition(nsh);

    literalTable.emplace_back();
    relocationTable.emplace_back();

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
        throw runtime_error("Assembler error: symbol was already defined!");
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
        throw runtime_error("Assembler error: symbol was already defined!");
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
        if (ELF32_ST_TYPE(sd->st_info) == STT_SECTION) {
            cerr << "Warning: section is global by default!" << endl;
            return;
        }

        sd->st_info = ELF32_ST_INFO(STB_GLOBAL, ELF32_ST_TYPE(sd->st_info));
    } else {
        Elf32_Sym nsd;
        nsd.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE); // everything else is default

        symbolTable.insertSymbolDefinition(nsd, symbol);
    }
}

void Assembler::addSymbolUsage(const string &symbol) {
    if (pass == 2 || currentSection == SHN_UNDEF || symbolTable.get(symbol) != nullptr) {
        return;
    }

    Elf32_Sym sd{}; // globalDef is false at this point

    symbolTable.insertSymbolDefinition(sd, symbol);
}

void Assembler::registerSection(const string &section) {
    if (pass == 1) {
        insertSection(section);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        currentSection++; // next section
        locationCounter = 0; // reset location counter

        Elf32_Shdr &sh = sectionTable.get(currentSection);
        sh.sh_offset = outputFile.tellp(); // set offset to current location
    }
}

void Assembler::insertSection(const string &section) {
    // open new literal table and relocation table for current section
    literalTable.emplace_back();
    relocationTable.emplace_back();

    currentSection++;

    // close last section
    sectionTable.closeLastSection(locationCounter);

    // reset location counter
    locationCounter = 0;

    // insert as section
    Elf32_Shdr &sd = sectionTable.insertSectionDefinition();

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
        sectionTable.closeLastSection(locationCounter);
    } else if (pass == 2) {
        initCurrentSectionPoolConstants();
        writeToOutputFile();
        readOutputFile();
    }
}

ostream &operator<<(ostream &os, const Assembler &as) {
    os << endl << as.sectionTable << endl << as.symbolTable << endl;
    for (const auto &it: as.relocationTable) {
        os << it << endl;
    }
    return os;
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

void Assembler::initSpaceWithConstant(const string &symbol) {
    // to be called with .word or at the end of the second pass for the current section, to init all pool values
    if (pass == 1) {
        addSymbolUsage(symbol);
    } else {
        // generate relocation
        generateRelocation(symbol);
    }
    incLocationCounter();
}

void Assembler::initSpaceWithConstant(Elf32_Word literal) {
    // to be called with .word or at the end of the second pass for the current section, to init all pool values
    if (pass == 2) {
        // generate no relocation
        outputFile.write(reinterpret_cast<char *>(&literal), sizeof(Elf32_Word));
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
        outputFile << ascii;
    }
}

void Assembler::incLocationCounter(Elf32_Word bytes) {
    locationCounter += bytes;
}

void Assembler::prepareSecondPass() {
    outputFile.open(outFile, ios::out | ios::binary);

    outputFile.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));
}

void Assembler::generateRelocation(const string &symbol) {
    Elf32_Sym *sd = symbolTable.get(symbol);

    if (ELF32_ST_BIND(sd->st_info) == STB_GLOBAL) {
        generateAbsoluteRelocation(sd);
    } else {
        generateRelativeRelocation(sd);
    }
}

void Assembler::generateAbsoluteRelocation(Elf32_Sym *sd) {
    // for generating R_32S
    Elf32_Rela rd;

    rd.r_addend = 0;
    rd.r_info = ELF32_R_INFO(sd->st_name, R_32S);
    rd.r_offset = locationCounter;

    auto &currentRelaTable = relocationTable[currentSection];
    currentRelaTable.insertRelocationEntry(rd);

    Elf32_Word relocationValue = sd->st_value;

    outputFile.write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
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

        rd.r_addend = -INSTRUCTION_LEN_BYTES;
        rd.r_info = ELF32_R_INFO(sd->st_name, R_PC32);
        rd.r_offset = locationCounter;

        auto &currentRelaTable = relocationTable[currentSection];
        currentRelaTable.insertRelocationEntry(rd);

        // offset to symbol address
        relocationValue = sd->st_value - locationCounter;
    }

    outputFile.write(reinterpret_cast<char *>(&relocationValue), sizeof(Elf32_Word));
}

void Assembler::zeroInitSpace(Elf32_Word bytes) {
    incLocationCounter(bytes);

    char buff[bytes];
    memset(buff, 0, sizeof(buff));
    outputFile.write(buff, bytes);
}

void Assembler::insertInstruction(yytokentype token, const vector<int16_t> &fields) {
    incLocationCounter(INSTRUCTION_LEN_BYTES);

    if (pass == 1) {
        return; // do nothing
    }

    Ins32 ins32 = Instruction32::getInstruction(token, fields);
    outputFile.write(reinterpret_cast<char *>(&ins32), sizeof(Ins32));
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

    Elf32_Shdr &section = sectionTable.get(currentSection);

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

void Assembler::writeSymbolTable(vector<Elf32_Shdr> &additionalHeaders) {
    // add symbol table section header
    Elf32_Shdr sh{};
    sh.sh_size = symbolTable.symbolDefinitions.size() * sizeof(Elf32_Sym);
    sh.sh_type = SHT_SYMTAB;
    sh.sh_link = SHT_STRTAB;
    // current position of file
    sh.sh_offset = outputFile.tellp();

    additionalHeaders.push_back(sh);

    // symtab
    for (auto &sd: symbolTable.symbolDefinitions) {
        outputFile.write(reinterpret_cast<char *>(&sd), sizeof(sd));
    }
}

void Assembler::writeStringTable(vector<Elf32_Shdr> &additionalHeaders) {
    // add symbol table section header
    Elf32_Shdr sh{};
    sh.sh_type = SHT_STRTAB;
    // current position of file
    sh.sh_offset = outputFile.tellp();


    // strtab
    for (auto &str: symbolTable.symbolNames) {
        const char *cst = str.c_str();
        outputFile.write(cst, static_cast<long>(strlen(cst) + 1)); // +1 for terminator
        sh.sh_size += strlen(cst) + 1;
    }

    additionalHeaders.push_back(sh);
}

void Assembler::writeRelocationTables(vector<Elf32_Shdr> &additionalHeaders) {
    // rela tables
    for (Elf32_Word sec = 0; sec < relocationTable.size(); sec++) {
        auto &relaTable = relocationTable[sec];

        // if no relocation entries skip
        if (relaTable.relocationEntries.empty()) {
            continue;
        }

        Elf32_Shdr sh{};
        sh.sh_size = relaTable.relocationEntries.size() * sizeof(Elf32_Rela);
        sh.sh_link = sec;
        sh.sh_info = SHT_RELA;
        sh.sh_offset = outputFile.tellp();

        // add relocation table section header
        additionalHeaders.push_back(sh);

        for (auto &rd: relaTable.relocationEntries) {
            outputFile.write(reinterpret_cast<char *>(&rd), sizeof(rd));
        }
    }
}

void Assembler::writeToOutputFile() {
    // what is written here: Elf Header, Sections
    // Need to do: symtab, strtab, rela

    vector<Elf32_Shdr> additionalHeaders;

    // write these first, before section header start
    writeRelocationTables(additionalHeaders);

    writeSymbolTable(additionalHeaders);

    writeStringTable(additionalHeaders);

    elfHeader.e_shoff = outputFile.tellp(); // section header offset
    cout << "Shoff: " << elfHeader.e_shoff << endl;

    // write main section headers
    for (auto &sh: sectionTable.sectionDefinitions) {
        cout << sh << endl;
        outputFile.write(reinterpret_cast<char *>(&sh), sizeof(sh));
    }

    // write additional section headers
    for (auto &sh: additionalHeaders) {
        outputFile.write(reinterpret_cast<char *>(&sh), sizeof(sh));
    }

    outputFile.seekp(SEEK_SET);
    outputFile.write(reinterpret_cast<char *>(&elfHeader), sizeof(Elf32_Ehdr));
}


void Assembler::readOutputFile() {
    cout << "Reading output..." << endl;

    ifstream file;
    file.open("a.out", ios::in | ios::binary);

    Elf32_Ehdr hdr;
    file.read(reinterpret_cast<char *>(&hdr), sizeof(Elf32_Ehdr));

    /*
    std::string s;
    std::getline(file, s, '\0');
     za dohvatanje stringova
     */

}
