BINARY=bin
CODEDIRS=./src/ ./misc/
INCDICRS=./inc/ ./misc/
OBJDIR=./out
DEPDIR=./out

CXX=g++
OPT=-O3
DEPFLAGS=-MP -MD
CXXFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

CXXFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))

OBJECTS=$(patsubst %.cpp,%.o,$(CXXFILES))
DEPFILES=$(patsubst %.cpp,%.d,$(CXXFILES))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(MAKE) -C ./misc
	$(CXX) -o $@ $^

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY) $(OBJECTS) $(DEPFILES)

distribute: clean
	tar zcvf dist.tgz *

diff:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	@git diff --stat

-include $(DEPFILES)

.PHONY: all clean distribute diff