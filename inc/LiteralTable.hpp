#pragma once

#include "Elf32.hpp"

#include <unordered_map>
#include <queue>

class LiteralTable {
public:
    void insertConstant(Elf32_Word literal, Elf32_Addr addr) {
        numericMap[literal] = addr;
    }

    void insertConstant(const string &symbol, Elf32_Addr addr) {
        symbolMap[symbol] = addr;
    }

    Elf32_Word getConstantAddress(Elf32_Word literal) {
        return numericMap[literal];
    }

    Elf32_Word getConstantAddress(const string &symbol) {
        return symbolMap[symbol];
    }

    bool hasConstant(Elf32_Word literal) {
        return numericMap.find(literal) != numericMap.end();
    }

    bool hasConstant(const string &symbol) {
        return symbolMap.find(symbol) != symbolMap.end();
    }

    Elf32_Word getNextLiteralConstante() {

    }

    int hasNextLocationValue() {
        if (!poolLiterals.empty() && poolLiterals.front().first < poolSymbols.front().first) {
            return 1;
        } else if (!poolSymbols.empty()) {
            return 2;
        } else {
            return 0;
        }
    }

private:
    std::unordered_map<Elf32_Word, Elf32_Word> numericMap;
    std::unordered_map<string, Elf32_Word> symbolMap;

    std::queue<pair<Elf32_Addr, Elf32_Word>> poolLiterals;
    std::queue<pair<Elf32_Addr, string>> poolSymbols;

    uint32_t iter = 0;

};

