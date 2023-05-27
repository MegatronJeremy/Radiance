#include "../../inc/emulator/Emulator.hpp"

void Emulator::handleInterrupts() {
    Elf32_Word cause = CAUSE_NO_INTR;

    // priority of serving interrupts: SOFTWARE INTR / BAD INSTRUCTION -> TIMER -> CONSOLE
    if (softwareIntrPending) {
        cause = CAUSE_INTR_SW;
        softwareIntrPending = false;
    } else if (badInstrIntrPending) {
        cause = CAUSE_BAD_INSTR;
        badInstrIntrPending = false;
    } else if (timerIntrPending && !interruptsMasked() && !timerMasked()) {
        cause = CAUSE_INTR_TIMER;
        timerIntrPending = false;
    } else if (terminalIntrPending && !interruptsMasked() && !terminalMasked()) {
        cause = CAUSE_INTR_TERMINAL;
        terminalIntrPending = false;
    }

    if (cause == CAUSE_NO_INTR) {
        return;
    }

    // setup interrupt routine
    push(cpu.getCSRX(STATUS));
    push(cpu.getGPRX(PC));
    cpu.setCSRX(CAUSE, cause);
    cpu.setCSRX(STATUS, cpu.getCSRX(STATUS) & (~0x1));
    cpu.setGPRX(PC, cpu.getCSRX(HANDLER));
}

void Emulator::writeToTerminal() {
    Elf32_Word ch = loadWordFromMem(TERM_OUT);

    putchar(static_cast<int>(ch));

    terminalWritePending = false;
}

void Emulator::handleTerminal() {
    // check for terminal interrupt
    int ch = getchar();
    if (ch != EOF) {
        // store new character to memory
        storeWordToMem(TERM_IN, ch);
        terminalIntrPending = true;
    }
}

void Emulator::handleTimer() {
    // end from last period start
    period_end = high_resolution_clock::now();

    // Decrement time left
    size_t timePassed = duration_cast<microseconds>(period_end - period_start).count();

    if (timePassed >= TIMER_MODES[timerMode]) {
        timerIntrPending = true;
        // start new period and generate interrupt
        period_start = high_resolution_clock::now();
    }
}
