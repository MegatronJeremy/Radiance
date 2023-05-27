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

    if (constant.isNumeric && positiveValueFitsInDisp(constant.number)) {
        insertInstruction(type, {R0, fields[REG_B], fields[REG_C], static_cast<int16_t>(constant.number)});
        return;
    }

    if (!constant.isNumeric) {
        Elf32_Sym *sd = eFile.symbolTable.get(constant.symbol);
        if (pass == 1 && sd->st_shndx == SHN_UNDEF) {
            // insert into jump table for size resolution after first pass
            JMPTabEntry jt;
            jt.address = locationCounter;
            jt.symbol = constant.symbol;

            size_t pad;
            if (type == BGT) pad = 2;
            else if (type != JMP) pad = 1;
            else pad = 0;
            jt.pad = pad * INSTRUCTION_LEN_BYTES;

            // don't insert for jmp - there is no pad
            if (jt.pad != 0) {
                jmpTabs[currentSection].push_back(jt);
            }
        }
        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            insertInstruction(type, {PC, fields[REG_B], fields[REG_C], disp});
            return;
        }
    }

    // code blocks checking jump condition using negation
    if (type == BEQ) {
        insertInstruction(BNE, {PC, fields[REG_B], fields[REG_C], 1 * INSTRUCTION_LEN_BYTES}); // skip over code block
    } else if (type == BNE) {
        insertInstruction(BEQ, {PC, fields[REG_B], fields[REG_C], 1 * INSTRUCTION_LEN_BYTES});
    } else if (type == BGT) {
        // check for two conditions to fulfill negation
        insertInstruction(BGT, {PC, fields[REG_C], fields[REG_B], 2 * INSTRUCTION_LEN_BYTES}); // invert registers
        insertInstruction(BEQ, {PC, fields[REG_B], fields[REG_C], 1 * INSTRUCTION_LEN_BYTES}); // skip block
    }

    Elf32_Addr poolConstantAddr = getPoolConstantAddr(constant);
    int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr,
                                                  locationCounter + WORD_LEN_BYTES); // one forward

    // pseudo-jump using load
    insertInstruction(LD_PCREL, {PC, R0, offsetToPoolLiteral});
}

void Assembler::insertCallIns(const PoolConstant &constant) {
    if (!constant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(constant.symbol);
    }

    if (constant.isNumeric && positiveValueFitsInDisp(constant.number)) {
        insertInstruction(CALL, {R0, R0, R0, static_cast<int16_t>(constant.number)});
        return;
    }

    if (!constant.isNumeric) {
        Elf32_Sym *sd = eFile.symbolTable.get(constant.symbol);
        if (pass == 1 && sd->st_shndx == SHN_UNDEF) {
            // insert into jump table for size resolution after first pass
            JMPTabEntry jt;
            jt.address = locationCounter;
            jt.symbol = constant.symbol;
            jt.pad = 4 * INSTRUCTION_LEN_BYTES;
            jmpTabs[currentSection].push_back(jt);
        }
        if ((pass == 1 && sd->st_shndx == SHN_UNDEF) || sd->st_shndx == currentSection) {
            int16_t disp = getDisplacement(sd->st_value, locationCounter + INSTRUCTION_LEN_BYTES);
            insertInstruction(CALL, {PC, R0, R0, disp});
            return;
        }
    }

    // insert long call
    int16_t stackR13Offs = -2 * WORD_LEN_BYTES;

    Elf32_Addr poolConstantAddr = getPoolConstantAddr(constant);

    insertInstruction(ST, {SP, R0, R13, stackR13Offs}); // store above top of stack (reserve one location)

    // push pc of instruction after jump, before jump
    insertInstruction(LD_REG, {R13, PC, 3 * INSTRUCTION_LEN_BYTES}); // skip over three instructions
    insertInstruction(PUSH, {R13}); // push return PC on reserved stack location
    stackR13Offs += WORD_LEN_BYTES; // because of push

    // restore R13
    insertInstruction(LD, {R13, SP, R0, stackR13Offs});

    int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr,
                                                  locationCounter + WORD_LEN_BYTES); // one forward
    // pseudo-jump using load
    insertInstruction(LD_PCREL, {PC, R0, offsetToPoolLiteral});
}

void Assembler::insertLoadIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (!poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    // will always pass all its register fields
    if (poolConstant.isNumeric && positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
        return;
    }

    // load with pool value
    Elf32_Addr poolConstantAddr = getPoolConstantAddr(PoolConstant{poolConstant.symbol});

    if (type == LD_REG) {
        // if ld_reg (load only immediate) this is all that is needed
        int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);
        fields.push_back(offsetToPoolLiteral);

        insertInstruction(LD_PCREL, fields);
    } else if (fields[REG_B] == R0 || fields[REG_A] != fields[REG_B]) {
        // use medium format with the destination register as the temporary register
        int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);

        insertInstruction(LD_PCREL, {fields[REG_A], R0, offsetToPoolLiteral});
        insertInstruction(type, {fields[REG_A], fields[REG_B], fields[REG_A]});
    } else {
        // long format with temporary register
        Reg regT = Instruction32::getNextGPR(fields[REG_B]);
        int16_t stackRTempOffs = -1 * WORD_LEN_BYTES;

        insertInstruction(ST, {SP, R0, regT, stackRTempOffs}); // store above top of stack

        // use an additional register to store symbol value
        int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);
        insertInstruction(LD_PCREL, {regT, R0, offsetToPoolLiteral});

        // regC is always free here
        insertInstruction(type, {fields[REG_A], fields[REG_B], regT});

        // restore RTemp
        insertInstruction(LD, {regT, SP, R0, stackRTempOffs});
    }
}

void Assembler::insertStoreIns(yytokentype type, const PoolConstant &poolConstant, vector<int16_t> &&fields) {
    if (!poolConstant.isNumeric) {
        // add symbol usage if in first pass
        addSymbolUsage(poolConstant.symbol);
    }

    if (poolConstant.isNumeric && positiveValueFitsInDisp(poolConstant.number)) {
        fields.push_back(static_cast<int16_t>(poolConstant.number));
        insertInstruction(type, fields);
        return;
    }

    Reg regT = Instruction32::getNextGPR(fields[REG_B]);

    int16_t stackRTempOffs = -1 * WORD_LEN_BYTES;
    insertInstruction(ST, {SP, R0, regT, stackRTempOffs}); // store above top of stack

    Elf32_Addr poolConstantAddr = getPoolConstantAddr(PoolConstant{poolConstant.symbol});
    int16_t offsetToPoolLiteral = getDisplacement(poolConstantAddr, locationCounter + WORD_LEN_BYTES);
    insertInstruction(LD_PCREL, {regT, R0, offsetToPoolLiteral});

    insertInstruction(type, {regT, fields[REG_B], fields[REG_C]}); // REG_A field is always free

    // restore RTemp
    insertInstruction(LD, {regT, SP, R0, stackRTempOffs});
}
