BUILD=./out
MKDIR_P=mkdir -p

TEST_DIR=./tests

AS_BUILD=$(BUILD)/src/assembler
AS_INC=./inc/assembler ./misc
AS_BIN=/usr/bin/assembler
AS_CODE=./src/assembler
AS_CXX=$(wildcard $(AS_CODE)/*.cpp)
AS_OBJ=$(patsubst $(AS_CODE)/%.cpp,$(AS_BUILD)/%.o,$(AS_CXX))
AS_DEP=$(patsubst $(AS_CODE)/%.cpp,$(AS_BUILD)/%.d,$(AS_CXX))

LD_BUILD=$(BUILD)/src/linker
LD_INC=./inc/linker ./inc/common
LD_BIN=/usr/bin/linker
LD_CODE=./src/linker
LD_CXX=$(wildcard $(LD_CODE)/*.cpp)
LD_OBJ=$(patsubst $(LD_CODE)/%.cpp,$(LD_BUILD)/%.o,$(LD_CXX))
LD_DEP=$(patsubst $(LD_CODE)/%.cpp,$(LD_BUILD)/%.d,$(LD_CXX))

RO_BUILD=$(BUILD)/src/readobj
RO_INC=./inc/common
RO_BIN=/usr/bin/readobj
RO_CODE=./src/readobj
RO_CXX=$(wildcard $(RO_CODE)/*.cpp)
RO_OBJ=$(patsubst $(RO_CODE)/%.cpp,$(RO_BUILD)/%.o,$(RO_CXX))
RO_DEP=$(patsubst $(RO_CODE)/%.cpp,$(RO_BUILD)/%.d,$(RO_CXX))

COM_BUILD=$(BUILD)/src/common
COM_INC=./inc/common
COM_CODE=./src/common
COM_CXX=$(wildcard $(COM_CODE)/*.cpp)
COM_OBJ=$(patsubst $(COM_CODE)/%.cpp,$(COM_BUILD)/%.o,$(COM_CXX))
COM_DEP=$(patsubst $(COM_CODE)/%.cpp,$(COM_BUILD)/%.d,$(COM_CXX))

MSC_BUILD=$(BUILD)/misc
MSC_INC=./misc
MSC_CODE=./misc
MSC_CXX=./misc/parser.cpp ./misc/lexer.cpp
MSC_OBJ=$(patsubst $(MSC_CODE)/%.cpp,$(MSC_BUILD)/%.o,$(MSC_CXX))
MSC_DEP=$(patsubst $(MSC_CODE)/%.cpp,$(MSC_BUILD)/%.d,$(MSC_CXX))

BUILD_DIRS=$(BUILD) $(AS_BUILD) $(LD_BUILD) $(COM_BUILD) $(RO_BUILD) $(MSC_BUILD)

CXX=g++
OPT=-O3
DEPFLAGS=-MP -MD
CXXFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

all: directories assembler readobj linker emulator

directories: $(BUILD_DIRS)

$(BUILD_DIRS):
	$(MKDIR_P) $@

assembler: $(AS_BIN)

readobj: $(RO_BIN)

linker: $(LD_BIN)

emulator:

$(AS_BIN):  $(MSC_OBJ) $(COM_OBJ) $(AS_OBJ)
	$(CXX) -o $@ $^

$(RO_BIN):  $(COM_OBJ) $(RO_OBJ)
	$(CXX) -o $@ $^

$(LD_BIN): $(COM_OBJ) $(LD_OBJ)
	$(CXX) -o $@ $^

$(BUILD)/src/assembler/%.o: $(AS_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/readobj/%.o: $(RO_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/linker/%.o: $(LD_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/misc/%.o: $(MSC_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/common/%.o: $(COM_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(MSC_CXX):
	$(MAKE) -C misc all

clean:
	$(MAKE) -C misc clean
	rm -rf out
	rm $(TEST_DIR)/*.o


-include $(AS_DEP) $(RO_DEP) $(LD_DEP) $(COM_DEP) $(MSC_DEP)

.PHONY: directories all assembler linker emulator clean
