#include "../inc/Assembler.hpp"
#include "../inc/Elf32File.hpp"

#include <iostream>
#include <memory>


using namespace std;

unique_ptr<Assembler> as;

extern void yyerror(const char *);

extern FILE *yyin;

int lineNum;

int assemble(int argc, char **argv) {
    if (argc < 3) {
        as = make_unique<Assembler>();
    } else {
        as = make_unique<Assembler>(argv[2]);
    }

    // open a file handle to a particular file:
    FILE *myfile = fopen(argv[1], "rw+");
    // make sure it's valid:
    if (!myfile) {
        cerr << "Can't open file!" << endl;
        return -1;
    }
    //Seek one character from the end of the file.
    fseek(myfile, -1, SEEK_END);

    //Read in a single character;
    char cLastChar = static_cast<char>(fgetc(myfile));

    if (cLastChar != '\n') {
        cerr << "Warning! Newline not found at the end of file, appending...\n";
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
    return 0;
}

int main(int argc, char **argv) {
    assemble(argc, argv);

    Elf32File elfFile{"a.out"};

    cout << elfFile << endl;

}