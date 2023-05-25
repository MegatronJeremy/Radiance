#pragma once

#include "../../inc/common/Elf32.hpp"

#include <vector>
#include <string>

using namespace std;

enum ValueSelector {
    OP,
    SYM,
    NUM
};

struct TNSEntry {
    void insertValue(char c) {
        selector.push_back(OP);
        operators.push_back(c);
    }

    void insertValue(const string &s) {
        selector.push_back(SYM);
        syms.push_back(s);
    }

    void insertValue(Elf32_Word num) {
        selector.push_back(NUM);
        nums.push_back(num);
    }

    bool resolved = false;

    vector<ValueSelector> selector;
    vector<char> operators;
    vector<string> syms;
    vector<Elf32_Word> nums;
};