CXXFILES=lexer.cpp parser.cpp
HXXFILES=lexer.hpp parser.hpp

all: $(CXXFILES) $(HXXFILES)

lexer.cpp: lexer.l
	flex lexer.l 

parser.cpp: parser.y lexer.l
	bison parser.y

clean: 
	rm -rf *.o *.d lexer.cpp lexer.hpp parser.cpp parser.hpp