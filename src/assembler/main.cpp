#include "../../inc/assembler/Assembler.hpp"

#include <iostream>
#include <memory>
#include <getopt.h>


using namespace std;

unique_ptr<Assembler> as;

extern void yyerror(const char *);

extern FILE *yyin;

int lineNum;

int main(int argc, char **argv) {

    string out_file = "a.out";

    int c;
    while ((c = getopt(argc, argv, "o:")) != -1) {
        switch (c) {
            case 'o':
                out_file = optarg;
                break;
            default:
                return -2;
        }
    }

    as = make_unique<Assembler>(out_file);

    if (optind >= argc) {
        cerr << "Assembler error: No input file named" << endl;
        return -3;
    }

    // open a file handle to a particular file (optind points to first positional argument):
    FILE *myfile = fopen(argv[optind], "rw+");
    // make sure it's valid:
    if (!myfile) {
        cerr << "Assembler error: No such file or directory" << endl;
        return -1;
    }
    //Seek one character from the end of the file.
    fseek(myfile, -1, SEEK_END);

    //Read in a single character;
    char cLastChar = static_cast<char>(fgetc(myfile));

    if (cLastChar != '\n') {
        cerr << "Warning! Newline not found at the end of file, appending\n";
        //Write the line-feed.
        fwrite("\n", sizeof(char), 1, myfile);
    }

    // Set lex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    while (as->nextPass()) {
        // fseek to start of file
        fseek(myfile, 0, SEEK_SET);

        // Reset line num
        lineNum = 1;

        // Parse through the input:
        try {
            yyparse();
        }
        catch (const exception &e) {
            yyerror(e.what());
            break;
        }
    }
}