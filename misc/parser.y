%{
  #include <iostream>
  #include <cstdio>
  #include "../inc/Assembler.hpp"
  #include "../inc/Ins32.hpp"
  using namespace std;

  extern int yylex();
  extern int lineNum;
  extern Assembler *as;

    void yyerror(const char*);
%}

%output  "parser.cpp"
%defines "parser.hpp"

%union {
    u_int32_t ival;
    u_int8_t rval;
    char *sval;
}


/* terminal symbols */
%token <ival> LITERAL
%token <sval> SYMBOL
%token <sval> STRING
%token <rval> GPRX CSRX

%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG
%token ADD SUB MUL DIV NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token ENDL

/* special symbols */
%token POP_CS

%type <ival> ival_expression
%type <ival> branch_operand

%left '+' '-'
%nonassoc UMINUS


%start program

%%

program:
    statements sentinel { as->endAssembly(); YYACCEPT; }

statements: 
    statement
    | statements statement
    ;

statement:
      label instruction ENDL
    | label directive ENDL
    | label ENDL
    | instruction ENDL
    | directive ENDL
    | ENDL
    ;

sentinel:
      ENDL
    | END;

label:
      SYMBOL ':' { as->insertLocalSymbol($1); free($1); }
    ;


directive:
      GLOBAL global_symbol_list
    | EXTERN extern_symbol_list
    | SECTION SYMBOL                 { as->registerSection($2); free($2); }
    | WORD initializer_list
    | SKIP LITERAL                   { as->zeroInitSpace($2); }
    | ASCII STRING                   { as->initAscii($2); free($2); }
    | EQU SYMBOL ',' ival_expression { as->insertAbsoluteSymbol($2, $4); free($2); }
    ;

instruction: 
      HALT                      { as->insertInstruction(HALT); }
    | INT                       { as->insertInstruction(INT); }
    | IRET                      { as->insertInstruction(IRET); }
    | CALL branch_operand
    | RET                       { as->insertInstruction(POP, {PC}); }
    | JMP branch_operand
    | BEQ branch_expression
    | BNE branch_expression
    | BGT branch_expression
    | PUSH GPRX                 { as->insertInstruction(PUSH, {$2}); }
    | POP GPRX                  { as->insertInstruction(POP, {$2}); }
    | XCHG GPRX ',' GPRX        { as->insertInstruction(XCHG, {0, $4, $2}); }
    | ADD GPRX ',' GPRX         { as->insertInstruction(ADD, {$4, $4, $2}); }
    | SUB GPRX ',' GPRX         { as->insertInstruction(SUB, {$4, $4, $2}); }
    | MUL GPRX ',' GPRX         { as->insertInstruction(MUL, {$4, $4, $2}); }
    | DIV GPRX ',' GPRX         { as->insertInstruction(DIV, {$4, $4, $2}); }
    | NOT GPRX                  { as->insertInstruction(NOT, {$2, $2}); }
    | AND GPRX ',' GPRX         { as->insertInstruction(AND, {$4, $4, $2}); }
    | OR GPRX ',' GPRX          { as->insertInstruction(OR, {$4, $4, $2}); }
    | XOR GPRX ',' GPRX         { as->insertInstruction(XOR, {$4, $4, $2}); }
    | SHL GPRX ',' GPRX         { as->insertInstruction(SHL, {$4, $4, $2}); }
    | SHR GPRX ',' GPRX         { as->insertInstruction(SHR, {$4, $4, $2}); }
    | LD data_operand ',' GPRX
    | ST GPRX ',' data_operand
    | CSRRD CSRX ',' GPRX       { as->insertInstruction(CSRRD, {$2, $4}); }
    | CSRWR GPRX ',' CSRX       { as->insertInstruction(CSRWR, {$2, $4}); }
    ; 

initializer_list:
      SYMBOL                        { as->initSpaceWithConstant($1); free($1); }
    | LITERAL                       { as->initSpaceWithConstant($1); }
    | initializer_list ',' SYMBOL   { as->initSpaceWithConstant($3); free($3); }
    | initializer_list ',' LITERAL  { as->initSpaceWithConstant($3); }
    ;

global_symbol_list:
      SYMBOL                        { as->insertGlobalSymbol($1); free($1); }
    | global_symbol_list ',' SYMBOL { as->insertGlobalSymbol($3); free($3); }
    ;

extern_symbol_list: // .extern symbols are ignored, all undefined symbols are external
      SYMBOL                        { free($1); }
    | extern_symbol_list ',' SYMBOL { free($3); }
    ;

branch_expression:
      GPRX ',' GPRX ',' branch_operand
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
    | '$' SYMBOL  { as->addSymbolUsage($2); free($2); }
    | LITERAL
    | SYMBOL      { as->addSymbolUsage($1); free($1); }
    | GPRX
    | '[' GPRX ']'
    | '[' GPRX '+' LITERAL ']'
    | '[' GPRX '+' SYMBOL ']'  { as->addSymbolUsage($4); free($4); }
    ;

branch_operand:
      LITERAL
    | SYMBOL { $$ = as->addSymbolUsage($1); free($1); }
    ;

%%



void yyerror(const char *s) {
  cout << "Parse error on line " << lineNum << "!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}