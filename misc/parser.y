%{
  #include <iostream>
  #include <cstdio>
  #include "../inc/Assembler.hpp"
  using namespace std;

  extern int yylex();
  extern int lineNum;
  extern Assembler as;

	void yyerror(const char*);
%}

%output  "parser.c"
%defines "parser.h"

%union {
    u_int32_t ival;
    int regA;
    int regB;
    int csrA;
    int csrB;
    char *sval;
}


/* terminal symbols */
%token <ival> LITERAL
%token <sval> SYMBOL
%token <sval> STRING 

%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token CSRX GPRX
%token ENDL

%type <ival> ival_expression

%left '+' '-'
%nonassoc UMINUS


%start program

%%

program:
    statements sentinel { as.endAssembly(); YYACCEPT; }

statements: 
    statement
    | statements statement
    ;

statement:
      label instruction ENDL  { as.locationCounter += 4; }
    | label directive ENDL
    | label ENDL
    | instruction ENDL        { as.locationCounter += 4; }
    | directive ENDL
    | ENDL
    ;

sentinel:
      ENDL
    | END;

label:
      SYMBOL ':' { as.insertLocalSymbol($1); free($1); } 
    ;


directive:
      GLOBAL global_symbol_list
    | EXTERN extern_symbol_list
    | SECTION SYMBOL                 {as.registerSection($2); free($2);}
    | WORD initializer_list          {as.locationCounter += 4;}
    | SKIP LITERAL                   {as.locationCounter += $2;}
    | ASCII STRING                   {as.reserveSpace($2); free($2);}
    | EQU SYMBOL ',' ival_expression {as.insertAbsoluteSymbol($2, $4); free($2);}
    ;

instruction: 
      HALT
    | INT
    | IRET
    | CALL branch_operand
    | RET
    | JMP branch_operand
    | BEQ branch_expression
    | BNE branch_expression
    | BGT branch_expression
    | PUSH GPRX
    | POP GPRX
    | XCHG alu_expression
    | ADD alu_expression
    | SUB alu_expression
    | MUL alu_expression
    | DIV alu_expression
    | NOT GPRX
    | AND alu_expression
    | OR alu_expression
    | XOR alu_expression
    | SHL alu_expression
    | SHR alu_expression
    | LD data_operand ',' GPRX
    | ST GPRX ','data_operand 
    | CSRRD CSRX ',' GPRX
    | CSRWR GPRX ',' CSRX
    ; 

initializer_list:
      SYMBOL                        {as.addSymbolUsage($1); free($1);}
    | LITERAL
    | initializer_list ',' SYMBOL   {as.addSymbolUsage($3); free($3);}
    | initializer_list ',' LITERAL
    ;

global_symbol_list:
      SYMBOL                        {as.insertGlobalSymbol($1); free($1);}
    | global_symbol_list ',' SYMBOL {as.insertGlobalSymbol($3); free($3);} 
    ;

extern_symbol_list:
      SYMBOL                        {as.addSymbolUsage($1); free($1);}
    | extern_symbol_list ',' SYMBOL {as.addSymbolUsage($3); free($3);} 
    ;

branch_expression:
      GPRX ',' GPRX ',' branch_operand
    ;

alu_expression:
      GPRX ',' GPRX
    ;

ival_expression:
      LITERAL     /* default action { $$ = $1;}  */
    | SYMBOL  { cout << $1 << endl; free($1); } // TODO
    | ival_expression '+' ival_expression { $$ = $1 + $3; }
    | ival_expression '-' ival_expression { $$ = $1 - $3; }
    | '-' ival_expression %prec UMINUS    { $$ = -$2; }
    | '(' ival_expression ')'             { $$ = $2;  }
    ;

data_operand:
      '$' LITERAL 
    | '$' SYMBOL  {as.addSymbolUsage($2); free($2);}
    | LITERAL
    | SYMBOL      {as.addSymbolUsage($1); free($1);}
    | reg_val
    | '[' reg_val ']'
    | '[' reg_val '+' LITERAL ']'
    | '[' reg_val '+' SYMBOL ']'  {as.addSymbolUsage($4); free($4);}
    ;

branch_operand:
      LITERAL
    | SYMBOL {as.addSymbolUsage($1); free($1);}
    ;

reg_val:
      GPRX
    | CSRX
    ;

%%



void yyerror(const char *s) {
  cout << "Parse error on line " << lineNum << "!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}