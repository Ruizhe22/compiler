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
#include "asm_auxiliary.h"



class AsmGenerator{
public:
    AsmGenerator(std::stringstream &ss, std::ostream &os);
    void generateRiscv();
    std::shared_ptr<Function> currentFunction;
    std::unordered_map<std::string, std::shared_ptr<Function>> mapFunction;
private:
    std::stringstream &koopa;
    std::ostream &ofs;
    // 所有代码先输出到oss里，再输出到ofs里，方便调整顺序
    std::stringstream oss;
    void rawHandler(const koopa_raw_program_t &raw);
    void visitRawSlice(const koopa_raw_slice_t &slice);
    void visitRawFunction(const koopa_raw_function_t &func);
    void visitRawBlock(const koopa_raw_basic_block_t &bb);
    void visitRawValue(const koopa_raw_value_t &value);
    void retHandler(const koopa_raw_value_t &value);
    void integerHandler(const koopa_raw_value_t &value);
    void binaryHandler(const koopa_raw_value_t &value);
    void allocHandler(const koopa_raw_value_t &value);
    void loadHandler(const koopa_raw_value_t &value);
    void storeHandler(const koopa_raw_value_t &value);
    void branchHandler(const koopa_raw_value_t &value);
    void jumpHandler(const koopa_raw_value_t &value);
    void globalAllocHandler(const koopa_raw_value_t &value);
    void callHandler(const koopa_raw_value_t &value);
    void getElemPtrHandler(const koopa_raw_value_t &value);
    void getPtrHandler(const koopa_raw_value_t &value);
    std::string generateLw(int src_offset, std::string dstReg, std::string base = "fp");
    std::string generateSw(int dst_offset, std::string srcReg, std::string base = "fp");
    void generateAggregate(koopa_raw_value_t init);
    static int typeSize(koopa_raw_type_t ty);
};

#endif //COMPILER_GENERATE_ASM_H
