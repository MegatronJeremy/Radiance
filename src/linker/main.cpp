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
            {nullptr, 0,                       nullptr, 0}
    };

    string out_file = "a.out";
    vector<pair<string, Elf32_Word >> place_defs;


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
                sscanf(optarg, "%s@%x", s, &p);
                place_defs.emplace_back(s, p);
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
    if (optind >= argc) {
        cerr << "Linker error: No input files named" << endl;
        return -3;
    }
    vector<string> input_files;
    while (optind < argc) {
        input_files.emplace_back(argv[optind++]);
    }

    Linker ld{input_files, out_file, place_defs, hex};

}
