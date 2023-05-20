#include "../../inc/common/Elf32File.hpp"

#include <getopt.h>


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

    Elf32File file{argv[optind]};

    if (genOutput) {
        ofstream f;
        f.open(outFile, ios::out);
        f << file;
        f.close();
    } else {
        cout << file << endl;
    }

}