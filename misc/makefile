CC=g++
SRCS=parser.cpp lexer.cpp ../src/main.cpp ../src/SectionTable.cpp ../src/SymbolTable.cpp ../src/Assembler.cpp

all: $(SRCS)
	$(CC) $(SRCS) -lfl -o parser

lexer.cpp: lexer.l
	flex lexer.l 

parser.cpp: parser.y lexer.l
	bison parser.y

clean: 
	rm -rf *.o lexer.cpp lexer.hpp parser.cpp parser.hpp parser