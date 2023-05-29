#include "../../inc/emulator/Emulator.hpp"

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
            switch (mode) {
                case MODE_CALL:
                    push(cpu.getGPRX(PC));
                    cpu.setGPRX(PC, cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp);
                    break;
                case MODE_CALL_IND:
                    push(cpu.getGPRX(PC));
                    cpu.setGPRX(PC, loadWordFromMem(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp));
                    break;
                default:
                    badInstrIntrPending = true;
            }
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
                case MODE_BEQ_IND:
                    if (cpu.getGPRX(regB) == cpu.getGPRX(regC))
                        goto execIndJump;
                    break;
                case MODE_BNE_IND:
                    if (cpu.getGPRX(regB) != cpu.getGPRX(regC))
                        goto execIndJump;
                    break;
                case MODE_BGT_IND:
                    if (static_cast<Elf32_Sword>(cpu.getGPRX(regB)) > static_cast<Elf32_Sword>(cpu.getGPRX(regC)))
                        goto execIndJump;
                    break;
                case MODE_JMP_IND:
                execIndJump:
                    cpu.setGPRX(PC, loadWordFromMem(cpu.getGPRX(regA) + disp));
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
                    if (cpu.getGPRX(regC) == 0) {
                        throw invalid_argument("Divide by zero exception");
                    }
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
                case MODE_ST_MEMIND_REGIND_DSP:
                    storeWordToMem(loadWordFromMem(cpu.getGPRX(regA) + cpu.getGPRX(regB) + disp), cpu.getGPRX(regC));
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

