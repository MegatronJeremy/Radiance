%{
    #include "parser.hpp"
    #include <iostream>
    using namespace std;
    extern int lineNum;
    extern int yyerror(const char*);
    #include "../inc/Ins32.hpp"
%}

%option outfile="lexer.cpp" header-file="lexer.hpp"

%option noyywrap

/* directives */
GLOB  ".global"
EXT   ".extern"  
SEC   ".section"
WORD  ".word"
SKIP  ".skip"
ASCII ".ascii"
EQU   ".equ"
END   ".end"  

/* instructions */
HALT  "halt"
INT   "int"
IRET  "iret"
CALL  "call"
RET   "ret"
JMP   "jmp"
BEQ   "beq"
BNE   "bne"
BGT   "bgt"
PUSH  "push"
POP   "pop"
XCHG  "xchg"
ADD   "add"
SUB   "sub"
MUL   "mul"
DIV   "div"
NOT   "not"
AND   "and"
OR    "or"
XOR   "xor"
SHL   "shl"
SHR   "shr"
LD    "ld"
ST    "st"
CSRRD "csrrd"
CSRWR "csrwr"

/* registers */
GPRX    (%r1[0-5])|(%r[0-9])
SP  "%sp"
PC  "%pc"
CSR0 "%status"
CSR1 "%handler"
CSR2 "%cause"


/* regular expression */
LITERAL (0[Xx])?[0-9A-Fa-f]+
SYM     [A-Za-z_][A-Za-z0-9_]*
STR     \"(\\.|[^\"])*\"
COM     "#"[^\n\r]*
WS      [ \r\t]*

%%

{WS}        { /* skip whitespace */ }
{COM}       { /* also skip comments */ }
[-+()\[\]=.,:$]    { return yytext[0]; }
\n          { lineNum++; return ENDL; }

{GLOB}      {return GLOBAL;}
{EXT}       {return EXTERN;}
{SEC}       {return SECTION;}
{WORD}      {return WORD;}
{SKIP}      {return SKIP;}
{ASCII}     {return ASCII;}
{EQU}       {return EQU;}
{END}       {return END;}
{HALT}      {return HALT;}
{INT}       {return INT;}
{IRET}      {return IRET;}
{CALL}      {return CALL;}
{RET}       {return RET;}
{JMP}       {return JMP;}
{BEQ}       {return BEQ;}
{BNE}       {return BNE;}
{BGT}       {return BGT;}
{PUSH}      {return PUSH;}
{POP}       {return POP;}
{XCHG}      {return XCHG;}
{ADD}       {return ADD;}
{SUB}       {return SUB;}
{MUL}       {return MUL;}
{DIV}       {return DIV;}
{NOT}       {return NOT;}
{AND}       {return AND;}
{OR}        {return OR;}
{XOR}       {return XOR;}
{SHL}       {return SHL;}
{SHR}       {return SHR;}
{LD}        {return LD;} 
{ST}        {return ST;}
{CSRRD}     {return CSRRD;}
{CSRWR}     {return CSRWR;}

{GPRX}      {yylval.rval = atoi(yytext + 2); return GPRX;}
{SP}        {yylval.rval = SP; return GPRX;}
{PC}        {yylval.rval = PC; return GPRX;}
{CSR0}      {yylval.rval = STATUS; return CSRX;}
{CSR1}      {yylval.rval = HANDLER; return CSRX;}
{CSR2}      {yylval.rval = CAUSE; return CSRX;}

{LITERAL}	  {
              yylval.ival = strtol(yytext, NULL, 0);  
			        return LITERAL;
            }
{SYM}       {
              yylval.sval = strdup(yytext);
			        return SYMBOL;
            }
{STR}       {
              yylval.sval = strdup(yytext);
              return STRING;
            }

.           { /* Default rule */ yyerror("Syntax error, unmatched symbol!"); }

%%