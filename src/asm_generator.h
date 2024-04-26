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
#include "koopa.h"

class AsmGenerator{
public:
    AsmGenerator(std::stringstream &ss, std::ostream &os);
    void generateRiscv();

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
    // koopa var to register t0 - t6
    std::unordered_map<koopa_raw_value_t, std::string> mapAllocReg;
    // unalloc register
    std::set<std::string> setUnallocReg;
    void initRegSet();
    std::string allocReg(const koopa_raw_value_t &);
    std::string allocReg();
    void restoreReg(const std::string &);

};


#endif //COMPILER_GENERATE_ASM_H
