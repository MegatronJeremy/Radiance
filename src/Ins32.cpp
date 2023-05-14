#include "../inc/Ins32.hpp"

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

    int currentParameter = 0;

    for (field_setter setOrDefaultX: setOrDefaultParams) {

        uint8_t field = 0;
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
    static std::map<yytokentype, uint8_t> opcodeMap = {
            {HALT, HALT_OP},

            {INT,  INT_OP},

            {CALL, CALL_OP},

            {JMP,  JMP_OP},
            {BEQ,  JMP_OP},
            {BNE,  JMP_OP},
            {BGT,  JMP_OP},

            {XCHG, XCHG_OP},

            {ADD,  ALU_OP},
            {SUB,  ALU_OP},
            {MUL,  ALU_OP},
            {DIV,  ALU_OP},

            {NOT,  LOG_OP},
            {AND,  LOG_OP},
            {OR,   LOG_OP},
            {XOR,  LOG_OP},

            {SHL,  SHF_OP},
            {SHR,  SHF_OP},

            {ST,   ST_OP},
            {PUSH, ST_OP},

            {LD,   LD_OP},
            {POP,  LD_OP}
    };
    auto it = opcodeMap.find(token);
    if (opcodeMap.find(token) == opcodeMap.end()) {
        throw std::runtime_error("Couldn't find instruction opcode!");
    }

    setOpcode(ins32, it->second);
}

void Instruction32::setMode(yytokentype token, Ins32 &ins32) {
    static std::map<yytokentype, uint8_t> modeMap = {
            {JMP,  MODE_JMP},
            {BEQ,  MODE_BEQ},
            {BNE,  MODE_BNE},
            {BGT,  MODE_BGT},

            {ADD,  MODE_ADD},
            {SUB,  MODE_SUB},
            {MUL,  MODE_MUL},
            {DIV,  MODE_DIV},

            {NOT,  MODE_NOT},
            {AND,  MODE_AND},
            {OR,   MODE_OR},
            {XOR,  MODE_XOR},

            {SHL,  MODE_SHL},
            {SHR,  MODE_SHR},

            {ST,   MODE_ST_REGIND_DSP},
            {PUSH, MODE_ST_REGIND_PTR_UPD},

            {LD,   MODE_LD_GPR_CSR},
            {POP,  MODE_LD_REGIND_PTR_UPD},
    };

    auto it = modeMap.find(token);

    if (it == modeMap.end()) {
        setMode(ins32, 0u);
    } else {
        setMode(ins32, it->second);
    }
}

bool Instruction32::setOrDefaultRegA(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::map<yytokentype, Reg> regAMap = {
            {PUSH, SP}
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
    static std::map<yytokentype, Reg> regBMap = {
            {POP, SP}
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
    static std::map<yytokentype, Reg> regCMap = {

    };

    auto it = regCMap.find(token);

    if (it == regCMap.end()) {
        setRegB(ins32, field);
        return true;
    } else {
        setRegB(ins32, it->second);
        return false;
    }
}

bool Instruction32::setOrDefaultDisp(yytokentype token, Ins32 &ins32, int16_t field) {
    static std::map<yytokentype, int16_t> dispMap = {
            {PUSH, -4},
            {POP,  +4}
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


