%{
  #include <iostream>
#include <memory>
  #include "../inc/assembler/Assembler.hpp"
  using namespace std;

  extern int yylex();
  extern int lineNum;
  extern unique_ptr<Assembler> as;

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
%token NOP
%token POP_CS
%token LD_REG LD_PCREL LD_IMM LD_DSP ST_DSP
%token ST_IND
%token CALL_IND
%token JMP_IND BEQ_IND BNE_IND BGT_IND

%left '+' '-'
%nonassoc UMINUS

%start program

%%

program:
    | statements { as->endAssembly(); }

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
    | EQU SYMBOL ',' ival_expression { as->closeTNSEntry($2); free($2); }
    | END                            { as->endAssembly(); YYACCEPT; }
    ;

instruction:
      HALT                                  { as->insertInstruction(HALT); }
    | INT                                   { as->insertInstruction(INT); }
    | IRET                                  { as->insertIretIns(); }
    | CALL LITERAL                          { as->insertCallIns($2); }
    | CALL SYMBOL                           { as->insertCallIns($2); free($2); }
    | RET                                   { as->insertInstruction(POP, {PC}); }
    | JMP LITERAL                           { as->insertJumpIns(JMP, $2); }
    | JMP SYMBOL                            { as->insertJumpIns(JMP, $2); free($2); }
    | BEQ GPRX ',' GPRX ',' LITERAL         { as->insertJumpIns(BEQ, $6, {R0, $2, $4}); }
    | BEQ GPRX ',' GPRX ',' SYMBOL          { as->insertJumpIns(BEQ, $6, {R0, $2, $4}); free($6); }
    | BNE GPRX ',' GPRX ',' LITERAL         { as->insertJumpIns(BNE, $6, {R0, $2, $4}); }
    | BNE GPRX ',' GPRX ',' SYMBOL          { as->insertJumpIns(BNE, $6, {R0, $2, $4}); free($6); }
    | BGT GPRX ',' GPRX ',' LITERAL         { as->insertJumpIns(BGT, $6, {R0, $2, $4}); }
    | BGT GPRX ',' GPRX ',' SYMBOL          { as->insertJumpIns(BGT, $6, {R0, $2, $4}); free($6); }
    | PUSH GPRX                             { as->insertInstruction(PUSH, {$2}); }
    | POP GPRX                              { as->insertInstruction(POP, {$2}); }
    | XCHG GPRX ',' GPRX                    { as->insertInstruction(XCHG, {$4, $2}); }
    | ADD GPRX ',' GPRX                     { as->insertInstruction(ADD, {$4, $4, $2}); }
    | SUB GPRX ',' GPRX                     { as->insertInstruction(SUB, {$4, $4, $2}); }
    | MUL GPRX ',' GPRX                     { as->insertInstruction(MUL, {$4, $4, $2}); }
    | DIV GPRX ',' GPRX                     { as->insertInstruction(DIV, {$4, $4, $2}); }
    | NOT GPRX                              { as->insertInstruction(NOT, {$2, $2}); }
    | AND GPRX ',' GPRX                     { as->insertInstruction(AND, {$4, $4, $2}); }
    | OR GPRX ',' GPRX                      { as->insertInstruction(OR, {$4, $4, $2}); }
    | XOR GPRX ',' GPRX                     { as->insertInstruction(XOR, {$4, $4, $2}); }
    | SHL GPRX ',' GPRX                     { as->insertInstruction(SHL, {$4, $4, $2}); }
    | SHR GPRX ',' GPRX                     { as->insertInstruction(SHR, {$4, $4, $2}); }
    | LD '$' LITERAL ',' GPRX               { as->insertLoadIns(LD_REG, $3, {$5, R0}); }
    | LD '$' SYMBOL ',' GPRX                { as->insertLoadIns(LD_REG, $3, {$5, R0}); free($3); }
    | LD LITERAL ',' GPRX                   { as->insertLoadIns(LD, $2, {$4, R0, R0}); }
    | LD SYMBOL ',' GPRX                    { as->insertLoadIns(LD, $2, {$4, R0, R0}); free($2); }
    | LD GPRX ',' GPRX                      { as->insertInstruction(LD_REG, {$4, $2}); }
    | LD '[' GPRX ']' ',' GPRX              { as->insertInstruction(LD, {$6, $3}); }
    | LD '[' GPRX '+' LITERAL ']' ',' GPRX  { as->insertLoadIns(LD_DSP, $5, {$8, $3, R0}); }
    | LD '[' GPRX '+' SYMBOL ']' ',' GPRX   { as->insertLoadIns(LD_DSP, $5, {$8, $3, R0}); free($5); }
    | ST GPRX ',' LITERAL                   { as->insertStoreIns(ST, $4, {R0, R0, $2}); }
    | ST GPRX ',' SYMBOL                    { as->insertStoreIns(ST, $4, {R0, R0, $2}); free($4); }
    | ST GPRX ',' '[' GPRX ']'              { as->insertInstruction(ST, {$5, R0, $2}); }
    | ST GPRX ',' '[' GPRX '+' LITERAL ']'  { as->insertStoreIns(ST_DSP, $7, {R0, $5, $2}); }
    | ST GPRX ',' '[' GPRX '+' SYMBOL ']'   { as->insertStoreIns(ST_DSP, $7, {R0, $5, $2}); free($7); }
    | CSRRD CSRX ',' GPRX                   { as->insertInstruction(CSRRD, {$4, $2}); }
    | CSRWR GPRX ',' CSRX                   { as->insertInstruction(CSRWR, {$4, $2}); }
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

extern_symbol_list:
      SYMBOL                        { as->addSymbolUsage($1); free($1); }
    | extern_symbol_list ',' SYMBOL { as->addSymbolUsage($3); free($3); }
    ;

ival_expression:
      LITERAL { as->insertTNS($1); }
    | SYMBOL  { as->insertTNS($1); free($1); }
    | ival_expression '+' ival_expression { as->insertTNS('+'); }
    | ival_expression '-' ival_expression { as->insertTNS('-'); }
    | '-' ival_expression %prec UMINUS    { as->insertTNS('-'); }
    | '(' ival_expression ')'
    ;

%%

void yyerror(const char *s) {
  cout << "Parse error on line " << lineNum << "!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}