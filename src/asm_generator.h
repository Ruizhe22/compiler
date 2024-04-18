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
    void retHandler(const koopa_raw_return_t &inst);
    void integerHandler(const koopa_raw_integer_t &inst);
};


#endif //COMPILER_GENERATE_ASM_H
