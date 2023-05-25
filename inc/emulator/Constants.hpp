#pragma once

#include <cstddef>

typedef unsigned char BYTE;

constexpr size_t MEMORY_SIZE_BYTES = 1ull << 32;

constexpr size_t NUM_GPRX_REGS = 16;

constexpr size_t NUM_CSRX_REGS = 3;

constexpr Elf32_Addr TERM_OUT = 0xFFFFFF00;

constexpr Elf32_Addr TERM_IN = 0xFFFFFF04;

constexpr Elf32_Addr TIM_CFG = 0xFFFFFF10;

constexpr Elf32_Word NUM_TIM_MODES = 8;

constexpr size_t TIMER_MODES[NUM_TIM_MODES] = { // in micros
        500 * 1000, 1000 * 1000, 1500 * 1000, 2000 * 1000,
        5000 * 1000, 10000 * 1000, 30000 * 1000, 60000 * 1000
};

constexpr Elf32_Word START_TIM_CFG = 0x0;

constexpr Elf32_Addr START_PC = 0x40000000;

constexpr Elf32_Addr START_SP = 0xFFFFFF00;


enum {
    CAUSE_NO_INTR = 0,
    CAUSE_BAD_INSTR,
    CAUSE_INTR_TIMER,
    CAUSE_INTR_TERMINAL,
    CAUSE_INTR_SW
};