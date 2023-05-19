lexer.cpp: lexer.l
	flex lexer.l 

parser.cpp: parser.y lexer.l
	bison parser.y

clean: 
	rm -rf *.o lexer.cpp lexer.hpp parser.cpp parser.hpp