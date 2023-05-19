#include "../../inc/linker/Linker.hpp"

#include <utility>

Linker::Linker(const vector<string> &inputFiles, string outFile,
               const vector<pair<string, Elf32_Addr>> &placeDefs, bool hexMode) : inputFiles(inputFiles),
                                                                                  outFile(std::move(outFile)),
                                                                                  placeDefs(placeDefs),
                                                                                  hexMode(hexMode) {}
