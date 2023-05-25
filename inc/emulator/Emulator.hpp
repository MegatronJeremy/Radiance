#pragma once

#include <algorithm>
#include "../common/Elf32.hpp"
#include "../common/Elf32File.hpp"
#include "../common/Ins32.hpp"
#include "Terminal.hpp"
#include "Constants.hpp"
#include "CPU.hpp"

class Emulator {
public:
    explicit Emulator(const string &inFile);

    void run();

private:
    void loadProgramData();

    void handleInstruction(Elf32_Word nextInstruction);

    void storeWordToMem(Elf32_Addr addr, Elf32_Word word);

    Elf32_Word loadWordFromMem(Elf32_Addr addr);

    void push(Elf32_Word reg);

    Elf32_Word pop();

    bool timerMasked() {
        return cpu.getCSRX(STATUS) & (1u << 0);
    }

    bool terminalMasked() {
        return cpu.getCSRX(STATUS) & (1u << 1);
    }

    bool interruptsMasked() {
        return cpu.getCSRX(STATUS) & (1u << 2);
    }


    Elf32File execFile;

    BYTE *memory;

    CPU cpu;

    bool running = false;

    bool softwareIntrPending = false;

    bool terminalIntrPending = false;

    bool terminalWritePending = false;

    bool timerIntrPending = false;

    bool badInstrIntrPending = false;

    void handleTerminal();

    void writeToTerminal();

    void handleInterrupts();
};