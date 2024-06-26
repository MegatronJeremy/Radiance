#pragma once

#include <cstdint>

// must include here because of type traits?
#include <unordered_map>
#include <vector>

#include "../../misc/parser.hpp"

constexpr int32_t INSTRUCTION_LEN_BYTES = 4;
constexpr int32_t WORD_LEN_BYTES = 4;

constexpr int32_t MIN_DISP = ~(0x7FF);
constexpr int32_t MAX_DISP = 0x7FF;

/* Register enumerations */
enum {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

/* Special general purpose registers */
enum {
    SP = 14,
    PC = 15
};

/* Control and status registers */
enum {
    STATUS = 0, // status
    HANDLER,    // handler
    CAUSE       // cause
};

/* Instruction parameter fields */
enum {
    REG_A = 0,
    REG_B,
    REG_C,
    DISP
};

/* Instruction opcode enumerations */

enum {
    HALT_OP = 0b0000,
    INT_OP,
    CALL_OP,
    JMP_OP,
    XCHG_OP,
    ALU_OP,
    LOG_OP,
    SHF_OP,
    ST_OP,
    LD_OP
};


/* Instruction mode enumerations */

enum {
    MODE_CALL = 0b0000,
    MODE_CALL_IND
};

enum {
    MODE_JMP = 0b0000,
    MODE_BEQ,
    MODE_BNE,
    MODE_BGT,
    MODE_JMP_IND = 0b1000,
    MODE_BEQ_IND,
    MODE_BNE_IND,
    MODE_BGT_IND
};

enum {
    MODE_ADD = 0b0000,
    MODE_SUB,
    MODE_MUL,
    MODE_DIV
};

enum {
    MODE_NOT = 0b0000,
    MODE_AND,
    MODE_OR,
    MODE_XOR
};

enum {
    MODE_SHL = 0b0000,
    MODE_SHR = 0b0001
};

enum {
    MODE_ST_REGIND_DSP = 0b0000,
    MODE_ST_REGIND_PTR_UPD,
    MODE_ST_MEMIND_REGIND_DSP
};

enum {
    MODE_LD_GPR_CSR = 0b0000,
    MODE_LD_GPR_GPR_DSP,
    MODE_LD_REGIND_DSP,
    MODE_LD_REGIND_PTR_UPD,
    MODE_LD_CSR_GPR,
    MODE_LD_CSR_OR,
    MODE_LD_CSR_REGIND_DSP,
    MODE_LD_CSR_REGIND_PTR_UPD
};


typedef uint32_t Ins32;
typedef uint8_t Reg;

class Instruction32 {
public:

    // inserted fields: {REG_A, REG_B, REG_C, DISP}
    static Ins32 getInstruction(yytokentype token, const std::vector<int16_t> &fields);

    static yytokentype getIndMode(yytokentype token);

    static uint8_t extractOpCode(Ins32 val) {
        return val >> 28;
    }

    static uint8_t extractMode(Ins32 val) {
        return (val & 0x0f000000) >> 24;
    }

    static Reg extractRegA(Ins32 val) {
        return (val & 0x00f00000) >> 20;
    }

    static Reg extractRegB(Ins32 val) {
        return (val & 0x000f0000) >> 16;
    }

    static Reg extractRegC(Ins32 val) {
        return (val & 0x0000f000) >> 12;
    }

    static Reg getNextGPR(Reg reg) {
        return reg % 11 + 3; // GPR 3 - 13 are available for assembler use
    }

    static int16_t extractDisp(Ins32 val) {
        uint16_t extracted = val & 0x0fff;
        if (extracted & 0x800) {  // MSB is set, value is negative
            extracted |= 0xf000;  // sign-extend
        }
        return (int16_t) extracted;
    }

private:
    static void setOpcode(yytokentype token, Ins32 &ins32);

    static void setMode(yytokentype token, Ins32 &ins32);

    // field - default, return true if it was set to passed field value

    static bool setOrDefaultRegA(yytokentype token, Ins32 &ins32, int16_t field);

    static bool setOrDefaultRegB(yytokentype token, Ins32 &ins32, int16_t field);

    static bool setOrDefaultRegC(yytokentype token, Ins32 &ins32, int16_t field);

    static bool setOrDefaultDisp(yytokentype token, Ins32 &ins32, int16_t field);

    // helper functions

    static void setOpcode(Ins32 &val, uint8_t opcode) {
        val = (val & 0x0fffffff) | (opcode << 28);
    }

    static void setMode(Ins32 &val, uint8_t mod) {
        val = (val & 0xf0ffffff) | (mod << 24);
    }

    static void setRegA(Ins32 &val, Reg op_a) {
        val = (val & 0xff0fffff) | (op_a << 20);
    }

    static void setRegB(Ins32 &val, Reg op_b) {
        val = (val & 0xfff0ffff) | (op_b << 16);
    }

    static void setRegC(Ins32 &val, Reg op_c) {
        val = (val & 0xffff0fff) | (op_c << 12);
    }

    static void setDisp(Ins32 &val, int16_t op_d) {
        val = (val & 0xfffff000) | (op_d & 0xfff);
    }

    using field_setter = bool (*)(yytokentype, Ins32 &, int16_t);

    static const field_setter setOrDefaultParams[];

    static constexpr uint32_t NUM_PARAMETERS = 4;
};