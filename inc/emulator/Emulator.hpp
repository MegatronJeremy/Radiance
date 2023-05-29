#pragma once

#include <algorithm>

#include <chrono>

using namespace std::chrono;

#include "../common/Elf32.hpp"
#include "../common/elf32file/Elf32File.hpp"
#include "../common/Ins32.hpp"
#include "NonBufferedTerminal.hpp"
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
        return cpu.getCSRX(STATUS) & TIMER_MASK_BIT;
    }

    bool terminalMasked() {
        return cpu.getCSRX(STATUS) & TERMINAL_MASK_BIT;
    }

    bool interruptsMasked() {
        return cpu.getCSRX(STATUS) & GLOBAL_MASK_BIT;
    }

    void writeToTerminal();

    void handleTerminal();

    void handleInterrupts();

    void handleTimer();

    Elf32File execFile;

    BYTE *memory;

    CPU cpu;

    bool running = false;

    bool terminalIntrPending = false;

    bool terminalWritePending = false;

    Elf32_Word timerMode = 0;

    time_point<high_resolution_clock> period_start = high_resolution_clock::now();

    time_point<high_resolution_clock> period_end;

    bool timerIntrPending = false;

    bool badInstrIntrPending = false;


};