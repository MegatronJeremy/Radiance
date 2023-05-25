CXXFILES=lexer.cpp parser.cpp

all: $(CXXFILES)

lexer.cpp: lexer.l
	flex lexer.l 

parser.cpp: parser.y lexer.l
	bison parser.y

clean: 
	rm -rf *.o *.d lexer.cpp lexer.hpp parser.cpp parser.hpp
