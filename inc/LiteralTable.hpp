#pragma once

#include "Elf32.hpp"

#include <unordered_map>
#include <queue>

class LiteralTable {
public:
    void insertLiteral(Elf32_Word literal, Elf32_Addr addr) {
        numericMap[literal] = addr;
    }

    void insertLiteral(const string &symbol, Elf32_Addr addr) {
        symbolMap[symbol] = addr;
    }

    void insertLocationValue(Elf32_Word val) {
        poolValues.push(val);
    }

    Elf32_Word getLiteralAddress(Elf32_Word literal) {
        return numericMap[literal];
    }

    Elf32_Word getLiteralAddress(const string &symbol) {
        return symbolMap[symbol];
    }

    bool hasLiteral(Elf32_Word literal) {
        return numericMap.find(literal) != numericMap.end();
    }

    bool hasLiteral(const string &symbol) {
        return symbolMap.find(symbol) != symbolMap.end();
    }

    Elf32_Word getNextLocationValue() {
        Elf32_Word ret = poolValues.front();
        poolValues.pop();
        return ret;
    }

    bool hasNextLocationValue() {
        return !poolValues.empty();
    }

private:
    std::unordered_map<Elf32_Word, Elf32_Word> numericMap;
    std::unordered_map<string, Elf32_Word> symbolMap;

    std::queue<Elf32_Word> poolValues;

    uint32_t iter = 0;

};

