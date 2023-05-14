#include "../inc/Assembler.hpp"

#include <iostream>
#include <memory>

using namespace std;

unique_ptr<Assembler> as;

extern void yyerror(const char *);

extern FILE *yyin;

int lineNum;

int main(int, char **) {
    as = make_unique<Assembler>();

    while (as->nextPass()) {
        // open a file handle to a particular file:
        FILE *myfile = fopen("sample.s", "r");
        // make sure it's valid:
        if (!myfile) {
            cout << "Can't open file!" << endl;
            return -1;
        }
        // Set lex to read from it instead of defaulting to STDIN:
        yyin = myfile;

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