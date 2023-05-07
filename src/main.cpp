#include "../inc/Assembler.hpp"

#include <iostream>

using namespace std;

Assembler *as;

extern int yyparse();

extern void yyerror(const char *);

extern FILE *yyin;

int main(int, char **) {
    as = new Assembler();

    if (as->nextPass()) {
        // open a file handle to a particular file:
        FILE *myfile = fopen("predavanja.s", "r");
        // make sure it's valid:
        if (!myfile) {
            cout << "Can't open file!" << endl;
            return -1;
        }
        // Set lex to read from it instead of defaulting to STDIN:
        yyin = myfile;

        // Parse through the input:
        try {
            yyparse();
        }
        catch (const exception &e) {
            yyerror(e.what());
        }
    }

    cout << *as << endl;

    delete as;
}