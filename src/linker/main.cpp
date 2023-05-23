#include "../../inc/linker/Linker.hpp"

#include <iostream>
#include <memory>
#include <getopt.h>


using namespace std;


int main(int argc, char **argv) {


    static struct option long_options[] = {
            {"place",       required_argument, nullptr, 'p'},
            {"hex",         no_argument,       nullptr, 'h'},
            {"relocatable", no_argument,       nullptr, 'r'},
            {nullptr,       0,                 nullptr, 0}
    };

    string out_file = "a.hex";
    unordered_map<string, Elf32_Word> place_defs;

    bool hex = false, rela = false;
    int c;
    while ((c = getopt_long(argc, argv, "o:p:hr", long_options, nullptr)) != -1) {
        switch (c) {
            case 'o':
                out_file = optarg;
                break;
            case 'p':
                char s[25];
                Elf32_Word p;
                char at;
                sscanf(optarg, "%[^@\n]%c%x", s, &at, &p);
                if (at != '@') {
                    cout << "Linker error: invalid format, place format is place=section@address";
                }
                place_defs[s] = p;
                break;
            case 'h':
                if (rela) {
                    cerr << "Linker error: Name only one option from relocatable or hex!" << endl;
                    return -3;
                }
                hex = true;
                break;
            case 'r':
                if (hex) {
                    cerr << "Linker error: Name only one option from relocatable or hex!" << endl;
                    return -3;
                }
                rela = true;
                break;
            default:
                return -2;
        }
    }
    if (!hex && !rela) {
        cerr << "Linker error: Must name either --relocatable or --hex!" << endl;
        return -4;
    }
    if (optind >= argc) {
        cerr << "Linker error: No input files named" << endl;
        return -5;
    }
    vector<string> input_files;
    while (optind < argc) {
        string file = argv[optind];
        input_files.emplace_back(file);
        optind++;
    }


    try {
        Linker ld{input_files, out_file, place_defs, hex};

        ld.run();
    } catch (exception &e) {
        cerr << e.what() << endl;
        return -1;
    }

    cout << "done" << endl;

}
