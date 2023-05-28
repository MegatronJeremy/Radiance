#include "../../inc/common/elf32file/Elf32File.hpp"

#include <getopt.h>
#include <memory>


int main(int argc, char **argv) {
    int c;
    bool genOutput = false;
    string outFile;

    while ((c = getopt(argc, argv, "o:")) != -1) {
        switch (c) {
            case 'o':
                genOutput = true;
                outFile = optarg;
                break;
            default:
                return -2;
        }
    }

    if (optind >= argc) {
        cerr << "Readobj error: no input file named" << endl;
        return -3;
    }

    unique_ptr<Elf32File> eFile;
    try {
        eFile = make_unique<Elf32File>();
        eFile->loadFromInputFile(argv[optind]);
    } catch (exception &e) {
        cout << e.what() << endl;
        return -1;
    }

    if (genOutput) {
        ofstream f;
        f.open(outFile, ios::out);
        f << *eFile;
        f.close();
    } else {
        cout << *eFile << endl;
    }
}