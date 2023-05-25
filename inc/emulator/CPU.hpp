#pragma once

#include <ostream>
#include <iomanip>
#include "../common/Elf32.hpp"
#include "Constants.hpp"

struct CPU {
public:
    Elf32_Word getGPRX(Reg reg) {
        if (reg >= NUM_GPRX_REGS) throw std::out_of_range("Gprx out of range");
        return GPRX[reg];
    }

    Elf32_Word getCSRX(Reg reg) {
        if (reg >= NUM_CSRX_REGS) throw std::out_of_range("Csrx out of range");
        return CSRX[reg];
    }

    void setGPRX(Reg reg, Elf32_Word val) {
        if (reg >= NUM_GPRX_REGS) throw std::out_of_range("Gprx out of range");
        GPRX[reg] = val;
    }

    void setCSRX(Reg reg, Elf32_Word val) {
        if (reg >= NUM_CSRX_REGS) throw std::out_of_range("Csrx out of range");
        CSRX[reg] = val;
    }

    friend ostream &operator<<(ostream &os, const CPU &cpu) {
        cout << "Emulated processor state:";
        for (size_t i = 0; i < NUM_GPRX_REGS; i++) {
            if (i % 4 == 0) {
                cout << endl;
            }
            cout << setw(3) << setfill(' ') << ("r" + to_string(i)) << "=0x" << setw(8) << setfill('0') << hex
                 << cpu.GPRX[i] << "\t";
        }
        return os;
    }

private:
    Elf32_Word GPRX[NUM_GPRX_REGS]{};

    Elf32_Word CSRX[NUM_CSRX_REGS]{};
};