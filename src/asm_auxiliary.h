//
// Created by ruizhe on 24-5-18.
//

#ifndef COMPILER_ASM_AUXILIARY_H
#define COMPILER_ASM_AUXILIARY_H
#include <string>
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <memory>
#include "koopa.h"

class Function{
public:
    Function(std::string namet);
    Function();
private:
    std::string name;
    // koopa var to register t0 - t6
    std::unordered_map<koopa_raw_value_t, std::string> mapAllocReg;
    // 相较于fp，为负数
    // unalloc register
    std::set<std::string> setUnallocReg;
    void initRegSet();

public:
    // 相较于fp，为负数
    std::unordered_map<koopa_raw_value_t, int> mapAllocMem;
    std::string allocReg(const koopa_raw_value_t &);
    std::string allocReg();
    void restoreReg(const std::string &);
    // for some value which is not unit, alloc memory, update mapAllocMem and esp
    int allocMem(const koopa_raw_value_t &);
    int sp;
};

class AsmTempLabel{
public:
    static AsmTempLabel globalLabel;
    int tempLabelCount = 0;
    std::string label();
    static std::string tempLabel();

};

#endif //COMPILER_ASM_AUXILIARY_H
