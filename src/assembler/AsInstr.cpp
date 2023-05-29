#include "../../inc/assembler/Assembler.hpp"
#include "../../inc/common/Util.hpp"

void Assembler::insertInstruction(yytokentype token, const vector<int16_t> &fields) {
    incLocationCounter(INSTRUCTION_LEN_BYTES);

    if (pass == 1) {
        return; // do nothing
    }

    Ins32 ins32 = Instruction32::getInstruction(token, fields);
    eFile.dataSections[currentSection].write(reinterpret_cast<char *>(&ins32), sizeof(Ins32));
}

void Assembler::insertIretIns() {
    insertInstruction(POP, {PC});
    insertInstruction(POP, {STATUS});
}

void Assembler::insertJumpIns(yytokentype type, const PoolConstant &constant,
                              const vector<int16_t> &fields) {
    if (!constant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(constant.symbol);
    }

    if (constant.isNumeric && Util::positiveValueFitsInDisp(constant.number)) {
        insertInstruction(type, {R0, fields[REG_B], fields[REG_C], static_cast<int16_t>(constant.number)});
        return;
    }

    if (!constant.isNumeric) {
        Elf32_Sym *sd = eFile.symbolTable.get(constant.symbol);

        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = Util::getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            insertInstruction(type, {PC, fields[REG_B], fields[REG_C], disp});
            return;
        }
    }

    Elf32_Addr poolConstantAddr = getPoolConstantAddr(constant);
    int16_t offsetToPoolLiteral = Util::getDisplacement(poolConstantAddr,
                                                        locationCounter + WORD_LEN_BYTES); // one forward

    // Indirect JMP or BRANCH
    insertInstruction(Instruction32::getIndMode(type), {PC, fields[REG_B], fields[REG_C], offsetToPoolLiteral});
}

void Assembler::insertCallIns(const PoolConstant &constant) {
    if (!constant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(constant.symbol);
    }

    if (constant.isNumeric && Util::positiveValueFitsInDisp(constant.number)) {
        insertInstruction(CALL, {R0, R0, R0, static_cast<int16_t>(constant.number)});
        return;
    }

    if (!constant.isNumeric) {
        Elf32_Sym *sd = eFile.symbolTable.get(constant.symbol);

        // check in second pass if symbol can be reached with PC relative addressing
        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = Util::getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            insertInstruction(CALL, {PC, R0, R0, disp});
            return;
        }
    }

    // indirect call needed with pool constant
    Elf32_Addr poolConstantAddr = getPoolConstantAddr(constant);

    int16_t offsetToPoolLiteral = Util::getDisplacement(poolConstantAddr,
                                                        locationCounter + WORD_LEN_BYTES); // one forward
    insertInstruction(CALL_IND, {PC, R0, R0, offsetToPoolLiteral});
}

void Assembler::insertLoadIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (!poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    // will always pass all its register fields
    if (poolConstant.isNumeric && Util::positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
        return;
    }

    if (!poolConstant.isNumeric) {
        // try to load symbol with PC relative addressing or as an absolute symbol
        Elf32_Sym *sd = eFile.symbolTable.get(poolConstant.symbol);

        if (pass == 1 && sd->st_shndx == SHN_UNDEF && !(type == LD_REG || type == LD_DSP)) {
            // padding for medium LD format
            IpadTabEntry it;
            it.address = locationCounter;
            it.symbol = poolConstant.symbol;
            it.pad = 1 * INSTRUCTION_LEN_BYTES;

            ipadTabs[currentSection].push_back(it);
        }

        // try to load absolute symbol into disp
        if ((type == LD_REG || type == LD_DSP) &&
            sd->st_shndx == SHN_ABS && Util::positiveValueFitsInDisp(sd->st_value)) {
            fields.push_back(static_cast<int16_t>(sd->st_value));
            insertInstruction(type, fields);
            return;
        }

        // load symbol relative to PC
        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = Util::getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            fields.push_back(disp);
            if (type == LD_REG) {
                insertInstruction(LD_REG, {fields[REG_A], PC, disp});
            } else {
                insertInstruction(type, {fields[REG_A], fields[REG_B], PC, disp});
            }
            return;
        }
    }

    // load with pool value
    Elf32_Addr poolConstantAddr = getPoolConstantAddr(poolConstant);

    switch (type) {
        case LD_REG: {
            // if ld_reg (load only immediate) this is all that is needed
            int16_t offsetToPoolLiteral = Util::getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);
            fields.push_back(offsetToPoolLiteral);

            insertInstruction(LD_PCREL, fields); // using REG_B (R0 here)
            break;
        }
        case LD_DSP: {
            // error! final symbol value unknown or can't fit into 12 bits!
            throw runtime_error("Assembler error: memind addressing type with value that cannot fit into displacement");
        }
        default: {
            // use medium format with the destination register as the temporary register
            int16_t offsetToPoolLiteral = Util::getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);

            insertInstruction(LD_PCREL, {fields[REG_A], R0, offsetToPoolLiteral});
            insertInstruction(type, {fields[REG_A], R0, fields[REG_A]});
        }
    }
}

void Assembler::insertStoreIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (!poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    if (poolConstant.isNumeric && Util::positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
        return;
    }

    if (!poolConstant.isNumeric) {
        Elf32_Sym *sd = eFile.symbolTable.get(poolConstant.symbol);

        // try to load absolute symbol into disp
        if (sd->st_shndx == SHN_ABS && Util::positiveValueFitsInDisp(sd->st_value)) {
            fields.push_back(static_cast<int16_t>(sd->st_value));
            insertInstruction(type, fields);
            return;
        }

        // try to use offset to a symbol with PC relative addressing
        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = Util::getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            insertInstruction(type, {PC, fields[REG_B], fields[REG_C], disp}); // REG_A field is always free + PC disp
            return;
        }
    }

    // else pool needed
    if (type != ST_DSP) {
        Elf32_Addr poolConstantAddr = getPoolConstantAddr(poolConstant);
        int16_t offsetToPoolLiteral = Util::getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);

        insertInstruction(ST_IND, {PC, R0, fields[REG_C], offsetToPoolLiteral});
    } else {
        // error! final symbol value unknown or can't fit into 12 bits!
        throw runtime_error("Assembler error: memind addressing type with value that cannot fit into displacement");
    }
}
