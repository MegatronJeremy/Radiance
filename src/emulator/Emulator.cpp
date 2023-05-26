#include <sys/mman.h>

#include "../../inc/emulator/Emulator.hpp"

Emulator::Emulator(const string &inFile) {
    memory = static_cast<BYTE *>(mmap(nullptr, MEMORY_SIZE_BYTES,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                                      -1, 0));
    execFile.loadFromInputFile(inFile);

    if (execFile.elfHeader.e_type != ET_EXEC) {
        throw runtime_error("Emulator error: file is not an executable");
    }

    cpu.setGPRX(PC, START_PC);
    cpu.setGPRX(SP, START_SP);

    // load program sections
    loadProgramData();
}

void Emulator::loadProgramData() {
    Elf32_Section i = 1; // program sections are one-indexed
    for (auto &phdr: execFile.programTable.programDefinitions) {
        stringstream &dataSection = execFile.dataSections[i++];

        dataSection.read(reinterpret_cast<char *>(&memory[phdr.p_paddr]), phdr.p_memsz);
    }
}

void Emulator::run() {
    Terminal terminal{}; // init terminal
    running = true;

    while (running) {
        Elf32_Word nextInstruction = loadWordFromMem(cpu.getGPRX(PC));

        // update PC
        cpu.setGPRX(PC, cpu.getGPRX(PC) + WORD_LEN_BYTES);

        try {
            handleInstruction(nextInstruction);
        } catch (out_of_range &e) {
            badInstrIntrPending = true;
        }

        if (terminalWritePending) {
            writeToTerminal();
        }

        handleTerminal();

        handleTimer();

        handleInterrupts();
    }

    cout << endl << endl << "-----------------------------------------------------------------" << endl;
    cout << "Emulated processor executed halt instruction" << endl;
    cout << cpu << endl;
}

void Emulator::storeWordToMem(Elf32_Addr addr, Elf32_Word word) {
    if (addr >= TERM_OUT && addr < TERM_OUT + WORD_LEN_BYTES) {
        terminalWritePending = true;
    }

    if (addr >= TIM_CFG && addr < TIM_CFG + WORD_LEN_BYTES) {
        timerMode = word;
    }

    *reinterpret_cast<Elf32_Word *>(memory + addr) = word;
}

Elf32_Word Emulator::loadWordFromMem(Elf32_Addr addr) {
    return *reinterpret_cast<Elf32_Word *>(memory + addr);
}

void Emulator::push(Elf32_Word reg) {
    cpu.setGPRX(SP, cpu.getGPRX(SP) - WORD_LEN_BYTES);
    storeWordToMem(cpu.getGPRX(SP), reg);
}

Elf32_Word Emulator::pop() {
    Elf32_Word ret = loadWordFromMem(cpu.getGPRX(SP));
    cpu.setGPRX(SP, cpu.getGPRX(SP) + WORD_LEN_BYTES);
    return ret;
}

void Emulator::handleInstruction(Elf32_Word nextInstruction) {
    uint8_t opCode, mode;
    Reg regA, regB, regC;
    Elf32_Sword disp;
    Elf32_Word temp;

    opCode = Instruction32::extractOpCode(nextInstruction);
    mode = Instruction32::extractMode(nextInstruction);

    regA = Instruction32::extractRegA(nextInstruction);
    regB = Instruction32::extractRegB(nextInstruction);
    regC = Instruction32::extractRegC(nextInstruction);
    disp = Instruction32::extractDisp(nextInstruction);

    switch (opCode) {
        case HALT_OP:
            running = false;
            break;
        case INT_OP:
            softwareIntrPending = true;
            break;
        case CALL_OP:
            push(cpu.getGPRX(PC));
            cpu.setGPRX(PC, cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp);
            break;
        case JMP_OP:
            switch (mode) {
                case MODE_BEQ:
                    if (cpu.getGPRX(regB) == cpu.getGPRX(regC))
                        goto execJump;
                    break;
                case MODE_BNE:
                    if (cpu.getGPRX(regB) != cpu.getGPRX(regC))
                        goto execJump;
                    break;
                case MODE_BGT:
                    if (static_cast<Elf32_Sword>(cpu.getGPRX(regB)) > static_cast<Elf32_Sword>(cpu.getGPRX(regC)))
                        goto execJump;
                    break;
                case MODE_JMP:
                execJump:
                    cpu.setGPRX(PC, cpu.getGPRX(regA) + disp);
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;
        case XCHG_OP:
            temp = cpu.getGPRX(regB);
            cpu.setGPRX(regB, cpu.getGPRX(regC));
            cpu.setGPRX(regC, temp);
            break;
        case ALU_OP:
            switch (mode) {
                case MODE_ADD:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) + cpu.getGPRX(regC));
                    break;
                case MODE_SUB:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) - cpu.getGPRX(regC));
                    break;
                case MODE_MUL:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) * cpu.getGPRX(regC));
                    break;
                case MODE_DIV:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) / cpu.getGPRX(regC));
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;
        case LOG_OP:
            switch (mode) {
                case MODE_NOT:
                    cpu.setGPRX(regA, ~cpu.getGPRX(regB));
                    break;
                case MODE_AND:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) & cpu.getGPRX(regC));
                    break;
                case MODE_OR:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) | cpu.getGPRX(regC));
                    break;
                case MODE_XOR:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) ^ cpu.getGPRX(regC));
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;

        case SHF_OP:
            switch (mode) {
                case MODE_SHL:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) << cpu.getGPRX(regC));
                    break;
                case MODE_SHR:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) >> cpu.getGPRX(regC));
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;

        case ST_OP:
            switch (mode) {
                case MODE_ST_REGIND_DSP:
                    storeWordToMem(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp, cpu.getGPRX(regC));
                    break;
                case MODE_ST_REGIND_PTR_UPD:
                    cpu.setGPRX(regA, cpu.getGPRX(regA) + disp);
                    cpu.setGPRX(R0, 0); // in case it was modified
                    storeWordToMem(cpu.getGPRX(regA), cpu.getGPRX(regC));
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;
        case LD_OP:
            switch (mode) {
                case MODE_LD_GPR_CSR:
                    cpu.setGPRX(regA, cpu.getCSRX(regB));
                    break;
                case MODE_LD_GPR_GPR_DSP:
                    cpu.setGPRX(regA, cpu.getGPRX(regB) + disp);
                    break;
                case MODE_LD_REGIND_DSP:
                    cpu.setGPRX(regA, loadWordFromMem(cpu.getGPRX(regB) + cpu.getGPRX(regC) + disp));
                    break;
                case MODE_LD_REGIND_PTR_UPD:
                    cpu.setGPRX(regA, loadWordFromMem(cpu.getGPRX(regB)));
                    cpu.setGPRX(regB, cpu.getGPRX(regB) + disp);
                    break;
                case MODE_LD_CSR_GPR:
                    cpu.setCSRX(regA, cpu.getGPRX(regB));
                    break;
                case MODE_LD_CSR_OR:
                    cpu.setCSRX(regA, cpu.getCSRX(regB) | disp);
                    break;
                case MODE_LD_CSR_REGIND_DSP:
                    cpu.setCSRX(regA, loadWordFromMem(cpu.getGPRX(regB) + cpu.getGPRX(regC) + disp));
                    break;
                case MODE_LD_CSR_REGIND_PTR_UPD:
                    cpu.setCSRX(regA, loadWordFromMem(cpu.getGPRX(regB)));
                    cpu.setGPRX(regB, cpu.getGPRX(regB) + disp);
                    break;
                default:
                    badInstrIntrPending = true;
            }
            break;
        default:
            badInstrIntrPending = true;
    }
    // in case it was modified, reset it
    cpu.setGPRX(R0, 0);
}

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

    push(cpu.getCSRX(STATUS));
    push(cpu.getGPRX(PC));
    cpu.setCSRX(CAUSE, cause);
    cpu.setCSRX(STATUS, cpu.getCSRX(STATUS) & (~0x1));
    cpu.setGPRX(PC, cpu.getCSRX(HANDLER));
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

void Emulator::writeToTerminal() {
    Elf32_Word ch = loadWordFromMem(TERM_OUT);

    putchar(static_cast<int>(ch));

    terminalWritePending = false;
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