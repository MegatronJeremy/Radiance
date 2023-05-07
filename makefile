SHELL=/bin/bash

INC=inc
SRC=src
MSC=misc
SRCS=$(MSC)/parser.c $(MSC)/lexer.c $(SRC)/main.cpp $(SRC)/SectionTable.cpp $(SRC)/SymbolTable.cpp $(SRC)/Assembler.cpp
INCS=$(INC)/Assembler.hpp $(INC)/SectionTable.hpp $(INC)/SymbolTable.hpp
CC=g++

all: $(SRCS)
	$(CC) $(SRCS) -o assembler

clean:
	rm -rf assembler