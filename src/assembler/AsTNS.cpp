#include "../../inc/assembler/Assembler.hpp"

#include<stack>

void Assembler::closeTNSEntry(const string &symbol) {
    TNS[symbol] = currentTNSEntry;
    currentTNSEntry = {};
}

void Assembler::insertTNS(char c) {
    currentTNSEntry.insertValue(c);
}

void Assembler::insertTNS(const string &symbol) {
    currentTNSEntry.insertValue(symbol);
}

void Assembler::insertTNS(Elf32_Word literal) {
    currentTNSEntry.insertValue(literal);
}

void Assembler::resolveTNS() {
    Elf32_Word resolved;
    Elf32_Word resolvedTotal = 0;

    do {
        resolved = 0;

        for (auto &it: TNS) {
            auto &tnsEntry = it.second;
            if (tnsEntry.resolved) {
                continue;
            }

            unordered_map<Elf32_Section, Elf32_Sword> clsNdx;
            unordered_map<Elf32_Section, Elf32_Sym *> boundSym;
            Elf32_Word a, b;
            Elf32_Word res;

            Elf32_Sym *sd, *symB;

            int signB;

            char op;

            stack<Elf32_Word> s;
            stack<Elf32_Sym *> symS;

            bool valid = true;

            vector<size_t> indices = {0, 0, 0};

            for (auto &selector: tnsEntry.selector) {
                switch (selector) {
                    case OP:
                        signB = 1;

                        b = s.top();
                        symB = symS.top();
                        s.pop();
                        symS.pop();

                        op = tnsEntry.operators[indices[OP]];

                        if (!s.empty()) {
                            a = s.top();
                            s.pop();
                            symS.pop();


                            if (op == '+') {
                                res = a + b;
                            } else {
                                res = a - b;
                                signB = -1;
                            }
                        } else {
                            if (op == '-') {
                                res = -b;
                                signB = -1;
                            }
                        }

                        if (symB != nullptr && symB->st_shndx != SHN_ABS) {
                            clsNdx[symB->st_shndx] += signB;
                            boundSym[symB->st_shndx] = symB;
                        }

                        s.push(res);
                        symS.push(nullptr);

                        indices[OP]++;
                        break;
                    case NUM:
                        s.push(tnsEntry.nums[indices[NUM]]);
                        symS.push(nullptr);

                        indices[NUM]++;
                        break;
                    case SYM:
                        sd = eFile.symbolTable.get(tnsEntry.syms[indices[SYM]]);

                        if (sd == nullptr) {
                            valid = false;
                        } else {
                            // if first value add to classification table (with positive sign)
                            if (s.empty() && sd->st_shndx != SHN_ABS) {
                                clsNdx[sd->st_shndx]++;
                                boundSym[sd->st_shndx] = sd;
                            }
                            s.push(sd->st_value);
                            symS.push(sd);
                            indices[SYM]++;
                        }
                        break;
                }
                if (!valid)
                    break;

            }

            if (valid) {
                Elf32_Section sndx = SHN_ABS;
                Elf32_Sym *bSym = nullptr;
                for (auto &cls: clsNdx) {
                    if (cls.second != 0 && sndx == SHN_ABS) {
                        sndx = cls.first;
                        bSym = boundSym[cls.first];
                    } else if (cls.second != 0) {
                        throw runtime_error("Assembler error: invalid EQU directive, bad index classification");
                    }
                }

                Elf32_Sym *targetSym = eFile.symbolTable.get(it.first);
                // binding to external symbol, and exporting as global
                if (bSym != nullptr && targetSym != nullptr && bSym->st_shndx == SHN_UNDEF &&
                    ELF32_ST_BIND(targetSym->st_info) == STB_GLOBAL) {
                    throw runtime_error(
                            "Assembler error: invalid EQU directive, "
                            "cannot bind to externally defined symbol while exporting target symbol as global");
                }

                resolved++;
                resolvedTotal++;
                tnsEntry.resolved = true;

                if (sndx == SHN_ABS) {
                    insertAbsoluteSymbol(it.first, s.top());
                } else if (sndx != SHN_UNDEF) {
                    currentSection = sndx; // set current section to index of target symbol
                    locationCounter = s.top(); // set location counter as the result value
                    insertLocalSymbol(it.first);
                } else {
                    // insert relocation value only for symbol (in a specially defined table)
                    Elf32_Rela rd;
                    rd.r_addend = static_cast<Elf32_Sword>(s.top()); // the constant addend
                    rd.r_info = ELF32_R_INFO(bSym->st_name, R_32S);
                    rd.r_offset = 0; // to be configured based on usage
                    equExtRela[it.first] = rd;
                    insertAbsoluteSymbol(it.first, 0); // insert as local symbol only
                }
            }
        }
    } while (resolved != 0);

    if (resolvedTotal != TNS.size()) {
        throw runtime_error("Assembler error: cannot resolve all EQU directives");
    }
}

