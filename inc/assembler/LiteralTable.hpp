#pragma once

#include "../common/Elf32.hpp"

#include <unordered_map>
#include <queue>
#include <utility>

struct PoolConstant {
    PoolConstant(Elf32_Word num) : number(num), isNumeric(true) {}

    PoolConstant(string sym) : symbol(std::move(sym)), isNumeric(false) {}

    PoolConstant(char *sym) : symbol(sym), isNumeric(false) {}

    Elf32_Word number{};
    string symbol;

    bool isNumeric = true;
};

class LiteralTable {
public:
    void insertConstant(const PoolConstant &constant, Elf32_Addr addr) {
        if (constant.isNumeric) {
            numericMap[constant.number] = addr;
        } else {
            symbolMap[constant.symbol] = addr;
        }
        poolConstantsQueue.push(constant);
    }

    Elf32_Word getConstantAddress(const PoolConstant &constant) {
        if (constant.isNumeric) {
            return numericMap[constant.number];
        } else {
            return symbolMap[constant.symbol];
        }
    }

    bool hasConstant(const PoolConstant &constant) {
        if (constant.isNumeric) {
            return numericMap.find(constant.number) != numericMap.end();
        } else {
            return symbolMap.find(constant.symbol) != symbolMap.end();
        }
    }

    PoolConstant getNextPoolConstant() {
        PoolConstant ret = poolConstantsQueue.front();
        poolConstantsQueue.pop();
        return ret;
    }

    bool hasNextPoolConstant() {
        return !poolConstantsQueue.empty();
    }

private:
    std::unordered_map<Elf32_Word, Elf32_Word> numericMap;
    std::unordered_map<string, Elf32_Word> symbolMap;

    std::queue<PoolConstant> poolConstantsQueue;

};

