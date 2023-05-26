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

EM_BUILD=$(BUILD)/src/emulator
EM_INC=./inc/emulator ./inc/common
EM_BIN=/usr/bin/emulator
EM_CODE=./src/emulator
EM_CXX=$(wildcard $(EM_CODE)/*.cpp)
EM_OBJ=$(patsubst $(EM_CODE)/%.cpp,$(EM_BUILD)/%.o,$(EM_CXX))
EM_DEP=$(patsubst $(EM_CODE)/%.cpp,$(EM_BUILD)/%.d,$(EM_CXX))

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
MSC_CXX=$(wildcard $(MSC_CODE)/*.cpp)
MSC_OBJ=$(patsubst $(MSC_CODE)/%.cpp,$(MSC_BUILD)/%.o,$(MSC_CXX))
MSC_DEP=$(patsubst $(MSC_CODE)/%.cpp,$(MSC_BUILD)/%.d,$(MSC_CXX))

BUILD_DIR_AS=$(AS_BUILD) $(COM_BUILD) $(MSC_BUILD)
BUILD_DIR_RO=$(RO_BUILD) $(COM_BUILD)
BUILD_DIR_LD=$(LD_BUILD) $(COM_BUILD)
BUILD_DIR_EM=$(EM_BUILD) $(COM_BUILD)

CXX=g++
OPT=-O3
DEPFLAGS=-MP -MD
CXXFLAGS=-Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

all: assembler readobj linker emulator

$(AS_BUILD):
	$(MKDIR_P) $@

$(COM_BUILD):
	$(MKDIR_P) $@

$(MSC_BUILD):
	$(MKDIR_P) $@

$(RO_BUILD):
	$(MKDIR_P) $@

$(LD_BUILD):
	$(MKDIR_P) $@

$(EM_BUILD):
	$(MKDIR_P) $@

assembler: $(BUILD_DIR_AS) $(AS_BIN)

readobj: $(BUILD_DIR_RO) $(RO_BIN)

linker: $(BUILD_DIR_LD) $(LD_BIN)

emulator: $(BUILD_DIR_EM) $(EM_BIN)

$(AS_BIN): $(MSC_OBJ) $(COM_OBJ) $(AS_OBJ)
	$(CXX) -o $@ $^

$(LD_BIN): $(COM_OBJ) $(LD_OBJ)
	$(CXX) -o $@ $^

$(EM_BIN): $(COM_OBJ) $(EM_OBJ)
	$(CXX) -o $@ $^

$(RO_BIN): $(COM_OBJ) $(RO_OBJ)
	$(CXX) -o $@ $^

$(BUILD)/src/assembler/%.o: $(AS_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/linker/%.o: $(LD_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/emulator/%.o: $(EM_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/readobj/%.o: $(RO_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/misc/%.o: $(MSC_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/src/common/%.o: $(COM_CODE)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf out
	rm -rf $(TEST_DIR)/*.o $(TEST_DIR)/*.out $(TEST_DIR)/*.hex


-include $(AS_DEP) $(RO_DEP) $(LD_DEP) $(EM_DEP) $(COM_DEP) $(MSC_DEP)

.PHONY: all misc assembler linker emulator clean
