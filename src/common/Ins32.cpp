#include "../../inc/common/Ins32.hpp"

#include <stdexcept>

const Instruction32::field_setter Instruction32::setOrDefaultParams[] = {
        setOrDefaultRegA,
        setOrDefaultRegB,
        setOrDefaultRegC,
        setOrDefaultDisp
};

Ins32 Instruction32::getInstruction(yytokentype token, const std::vector<int16_t> &parameters) {
    Ins32 ins32 = 0u;

    // opcode and mode are never passed as parameters
    setOpcode(token, ins32);
    setMode(token, ins32);

    unsigned int currentParameter = 0;

    for (field_setter setOrDefaultX: setOrDefaultParams) {

        int16_t field = 0;
        if (currentParameter < parameters.size()) {
            field = parameters[currentParameter];
        }

        if (setOrDefaultX(token, ins32, field)) {
            currentParameter++;
        }
    }

    return ins32;
}


void Instruction32::setOpcode(yytokentype token, Ins32 &ins32) {
    static std::unordered_map<yytokentype, uint8_t> opcodeMap = {
            {HALT,       HALT_OP},

            {INT,        INT_OP},

            {CALL,       CALL_OP},
            {CALL_IND,   CALL_OP},

            {JMP,        JMP_OP},
            {BEQ,        JMP_OP},
            {BNE,        JMP_OP},
            {BGT,        JMP_OP},
            {JMP_IND,    JMP_OP},
            {BEQ_IND,    JMP_OP},
            {BNE_IND,    JMP_OP},
            {BGT_IND,    JMP_OP},

            {XCHG,       XCHG_OP},

            {ADD,        ALU_OP},
            {SUB,        ALU_OP},
            {MUL,        ALU_OP},
            {DIV,        ALU_OP},

            {NOT,        LOG_OP},
            {AND,        LOG_OP},
            {OR,         LOG_OP},
            {XOR,        LOG_OP},

            {SHL,        SHF_OP},
            {SHR,        SHF_OP},

            {ST,         ST_OP},
            {ST_DSP,     ST_OP},
            {PUSH,       ST_OP},
            {ST_IND,     ST_OP},

            {LD,         LD_OP},
            {LD_DSP,     LD_OP},
            {LD_REG,     LD_OP},
            {LD_PCREL,   LD_OP},
            {LD_DSP_CSR, LD_OP},
            {POP,        LD_OP},
            {POP_DSP,    LD_OP},
            {POP_CS,     LD_OP},
            {CSRRD,      LD_OP},
            {CSRWR,      LD_OP},

            {NOP,        LOG_OP}
    };
    auto it = opcodeMap.find(token);
    if (opcodeMap.find(token) == opcodeMap.end()) {
        throw std::runtime_error("Couldn't find instruction opcode!");
    }

    setOpcode(ins32, it->second);
}

void Instruction32::setMode(yytokentype token, Ins32 &ins32) {
    static std::unordered_map<yytokentype, uint8_t> modeMap = {
            {CALL,       MODE_CALL},
            {CALL_IND,   MODE_CALL_IND},

            {JMP,        MODE_JMP},
            {BEQ,        MODE_BEQ},
            {BNE,        MODE_BNE},
            {BGT,        MODE_BGT},
            {JMP_IND,    MODE_JMP_IND},
            {BEQ_IND,    MODE_BEQ_IND},
            {BNE_IND,    MODE_BNE_IND},
            {BGT_IND,    MODE_BGT_IND},

            {ADD,        MODE_ADD},
            {SUB,        MODE_SUB},
            {MUL,        MODE_MUL},
            {DIV,        MODE_DIV},

            {NOT,        MODE_NOT},
            {AND,        MODE_AND},
            {OR,         MODE_OR},
            {XOR,        MODE_XOR},

            {SHL,        MODE_SHL},
            {SHR,        MODE_SHR},

            {ST,         MODE_ST_REGIND_DSP},
            {ST_DSP,     MODE_ST_REGIND_DSP},
            {PUSH,       MODE_ST_REGIND_PTR_UPD},
            {ST_IND,     MODE_ST_MEMIND_REGIND_DSP},

            {LD,         MODE_LD_REGIND_DSP},
            {LD_DSP,     MODE_LD_REGIND_DSP},
            {LD_PCREL,   MODE_LD_REGIND_DSP},
            {LD_REG,     MODE_LD_GPR_GPR_DSP},
            {POP,        MODE_LD_REGIND_PTR_UPD},
            {POP_DSP,    MODE_LD_REGIND_PTR_UPD},
            {LD_DSP_CSR, MODE_LD_CSR_REGIND_DSP},
            {POP_CS,     MODE_LD_CSR_REGIND_PTR_UPD},
            {POP_CS_DSP, MODE_LD_CSR_REGIND_PTR_UPD},
            {CSRRD,      MODE_LD_GPR_CSR},
            {CSRWR,      MODE_LD_CSR_GPR},

            {NOP,        MODE_XOR}
    };

    auto it = modeMap.find(token);

    if (it == modeMap.end()) {
        setMode(ins32, 0u);
    } else {
        setMode(ins32, it->second);
    }
}

bool Instruction32::setOrDefaultRegA(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::unordered_map<yytokentype, Reg> regAMap = {
            {PUSH, SP},

            {XCHG, R0},

            {NOP,  R0}
    };

    auto it = regAMap.find(token);

    if (it == regAMap.end()) {
        setRegA(ins32, field);
        return true;
    } else {
        setRegA(ins32, it->second);
        return false;
    }
}

bool Instruction32::setOrDefaultRegB(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::unordered_map<yytokentype, Reg> regBMap = {
            {LD_PCREL,   PC},

            {PUSH,       R0},

            {POP,        SP},
            {POP_DSP,    SP},
            {POP_CS,     SP},
            {POP_CS_DSP, SP},

            {NOP,        R0}
    };

    auto it = regBMap.find(token);

    if (it == regBMap.end()) {
        setRegB(ins32, field);
        return true;
    } else {
        setRegB(ins32, it->second);
        return false;
    }
}

bool Instruction32::setOrDefaultRegC(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::unordered_map<yytokentype, Reg> regCMap = {
            {LD_REG,     R0},

            {POP_DSP,    R0},
            {POP_CS_DSP, R0},

            {NOP,        R0}
    };

    auto it = regCMap.find(token);

    if (it == regCMap.end()) {
        setRegC(ins32, field);
        return true;
    } else {
        setRegC(ins32, it->second);
        return false;
    }
}

bool Instruction32::setOrDefaultDisp(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::unordered_map<yytokentype, int16_t> dispMap = {
            {PUSH,   -4},
            {POP,    +4},
            {POP_CS, +4},
    };

    auto it = dispMap.find(token);

    if (it == dispMap.end()) {
        setDisp(ins32, field);
        return true;
    } else {
        setDisp(ins32, it->second);
        return false;
    }
}

yytokentype Instruction32::getIndMode(yytokentype token) {
    static std::unordered_map<yytokentype, yytokentype> modeMap = {
            {JMP, JMP_IND},
            {BEQ, BEQ_IND},
            {BNE, BNE_IND},
            {BGT, BGT_IND}
    };

    if (modeMap.find(token) == modeMap.end()) {
        throw std::runtime_error("Assembler error: indirect mode not found for instruction");
    }

    return modeMap[token];
}
