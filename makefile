BUILD=./out

AS_INC=./inc/assembler ./misc/
AS_BIN=/usr/bin/assembler
AS_CODE=./src/assembler
AS_CXX=$(wildcard $(AS_CODE)/*.cpp)
AS_OBJ=$(patsubst $(AS_CODE)/%.cpp,$(BUILD)/%.o,$(AS_CXX))
AS_DEP=$(patsubst $(AS_CODE)/%.cpp,$(BUILD)/%.d,$(AS_CXX))

COM_INC=./inc/common
COM_CODE=./src/common
COM_CXX=$(wildcard $(COM_CODE)/*.cpp)
COM_OBJ=$(patsubst $(COM_CODE)/%.cpp,$(BUILD)/%.o,$(COM_CXX))
COM_DEP=$(patsubst $(COM_CODE)/%.cpp,$(BUILD)/%.d,$(COM_CXX))

MSC_INC=./misc
MSC_CODE=./misc
MSC_CXX=./misc/parser.cpp ./misc/lexer.cpp
MSC_OBJ=$(patsubst $(MSC_CODE)/%.cpp,$(BUILD)/%.o,$(MSC_CXX))
MSC_DEP=$(patsubst $(MSC_CODE)/%.cpp,$(BUILD)/%.d,$(MSC_CXX))

CXX=g++
OPT=-O3
DEPFLAGS=-MP -MD
CXXFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

all: assembler linker emulator

assembler: $(AS_BIN)

$(AS_BIN):  $(MSC_OBJ) $(COM_OBJ) $(AS_OBJ)
	$(CXX) -o $@ $^

$(BUILD)/%.o: $(MSC_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(COM_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(AS_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(MSC_CXX):
	$(MAKE) -C misc all

linker:

emulator:

clean:
	$(MAKE) -C misc clean
	rm -rf out/*
	rm -rf $(MSC_TMP)

-include $(AS_DEP) $(COM_DEP) $(MSC_DEP)

.PHONY: all assembler linker emulator clean
