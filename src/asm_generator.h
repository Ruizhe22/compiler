//
// Created by ruizhe on 24-4-18.
//

#ifndef COMPILER_GENERATE_ASM_H
#define COMPILER_GENERATE_ASM_H

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
    int sp;
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
};


class AsmGenerator{
public:
    AsmGenerator(std::stringstream &ss, std::ostream &os);
    void generateRiscv();
    std::shared_ptr<Function> currentFunciton;
    std::unordered_map<std::string, std::shared_ptr<Function>> mapFunction;
private:
    std::stringstream &koopa;
    std::ostream &fos;
    void rawHandler(const koopa_raw_program_t &raw);
    void visitRawSlice(const koopa_raw_slice_t &slice);
    void visitRawFunction(const koopa_raw_function_t &func);
    void visitRawBlock(const koopa_raw_basic_block_t &bb);
    void visitRawValue(const koopa_raw_value_t &value);
    void retHandler(const koopa_raw_value_t &value);
    void integerHandler(const koopa_raw_value_t &value);
    void binaryHander(const koopa_raw_value_t &value);
    void allocHander(const koopa_raw_value_t &value);
    void loadHander(const koopa_raw_value_t &value);
    void storeHander(const koopa_raw_value_t &value);

};


#endif //COMPILER_GENERATE_ASM_H
