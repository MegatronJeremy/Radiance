#pragma once

#include <algorithm>

#include <chrono>

using namespace std::chrono;

#include "../common/Elf32.hpp"
#include "../common/Elf32File.hpp"
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
        return cpu.getCSRX(STATUS) & (1u << 0);
    }

    bool terminalMasked() {
        return cpu.getCSRX(STATUS) & (1u << 1);
    }

    bool interruptsMasked() {
        return cpu.getCSRX(STATUS) & (1u << 2);
    }

    void writeToTerminal();

    void handleTerminal();

    void handleInterrupts();

    void handleTimer();

    Elf32File execFile;

    BYTE *memory;

    CPU cpu;

    bool running = false;

    bool softwareIntrPending = false;

    bool terminalIntrPending = false;

    bool terminalWritePending = false;

    Elf32_Word timerMode = 0;

    time_point<high_resolution_clock> period_start = high_resolution_clock::now();

    time_point<high_resolution_clock> period_end;

    bool timerIntrPending = false;

    bool badInstrIntrPending = false;


};