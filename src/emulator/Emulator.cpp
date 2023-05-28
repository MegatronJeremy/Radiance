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
    NonBufferedTerminal terminal{}; // init terminal
    running = true;

    while (running) {
        Elf32_Word nextInstruction = loadWordFromMem(cpu.getGPRX(PC));

        cout << "Next instruction: " << hex << nextInstruction << endl;
        cout << "Current pc: " << hex << cpu.getGPRX(PC) << endl << endl;

        // update PC
        cpu.setGPRX(PC, cpu.getGPRX(PC) + WORD_LEN_BYTES);

        try {
            handleInstruction(nextInstruction);
        } catch (exception &e) {
            cout << e.what() << endl;
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

