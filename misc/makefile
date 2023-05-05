CC=g++
SRCS=parser.c lexer.c ../src/main.cpp ../src/SectionTable.cpp ../src/SymbolTable.cpp ../src/Assembler.cpp

all: $(SRCS)
	$(CC) $(SRCS) -lfl -o parser

lexer.c: lexer.l
	flex lexer.l 

parser.c: parser.y lexer.l
	bison parser.y

clean: 
	rm -rf *.o lexer.c lexer.h parser.c parser.h parser