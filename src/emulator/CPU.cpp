#include "../../inc/emulator/CPU.hpp"

#include<iostream>

ostream &operator<<(ostream &os, const CPU &cpu) {
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
