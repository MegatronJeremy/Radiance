SHELL=/bin/bash

INC=inc
SRC=src
SRCS=$(SRC)/Assembler.cpp
INCS=$(INC)/Assembler.hpp $(INC)/SectionTable.hpp $(INC)/SymbolTable.hpp
CC=g++

all: $(SRCS)
	$(CC) $(SRCS) -o assembler

clean:
	rm -rf assembler