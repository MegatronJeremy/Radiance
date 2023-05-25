#include "../../inc/emulator/Emulator.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Assembler error: No input file named" << endl;
        return -3;
    }

    try {
        Emulator em{argv[1]};
        em.run();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return -1;
    }

}