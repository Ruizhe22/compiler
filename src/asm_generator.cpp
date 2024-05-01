//
// Created by ruizhe on 24-4-18.
//

#include "asm_generator.h"

void AsmGenerator::generateRiscv()
{
    std::string koopa_string = koopa.str();
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(koopa_string.c_str(), &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    rawHandler(raw);

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);

}

AsmGenerator::AsmGenerator(std::stringstream &ss, std::ostream &os):koopa(ss), fos(os){}

void AsmGenerator::rawHandler(const koopa_raw_program_t &raw)
{
    // 执行一些其他的必要操作
    fos << "\t.text\n"
           "\t.globl main\n" ;
    // 访问所有全局变量
    visitRawSlice(raw.values);
    // 访问所有函数
    visitRawSlice(raw.funcs);

}

void AsmGenerator::visitRawSlice(const koopa_raw_slice_t &slice)
{
    switch (slice.kind) {
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            for (size_t i = 0; i < slice.len; ++i) {
                auto ptr = slice.buffer[i];
                visitRawFunction(reinterpret_cast<koopa_raw_function_t>(ptr));
            }
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            for (size_t i = 0; i < slice.len; ++i) {
                auto ptr = slice.buffer[i];
                visitRawBlock(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            }
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            for (size_t i = 0; i < slice.len; ++i) {
                auto ptr = slice.buffer[i];
                visitRawValue(reinterpret_cast<koopa_raw_value_t>(ptr));
            }
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);

    }
}


// 访问函数
void AsmGenerator::visitRawFunction(const koopa_raw_function_t &func)
{
    // 执行一些其他的必要操作
    currentFunciton = std::make_shared<Function>();
    if(func->name){
        std::string funcName = func->name + 1;
        fos << funcName << ":\n";
        mapFunction[funcName] = currentFunciton;
    }
    // 更新帧指针
    fos << "\tmv fp, sp" << "\n";
    // 访问所有基本块
    visitRawSlice(func->bbs);
}

// 访问基本块
void AsmGenerator::visitRawBlock(const koopa_raw_basic_block_t &bb)
{
    if(bb->name){
        std::string blockName = bb->name + 1;
        fos << blockName << ":\n";
    }
    // 访问所有指令
    visitRawSlice(bb->insts);
}

// 访问指令
void AsmGenerator::visitRawValue(const koopa_raw_value_t &value)
{
    // 读取一条指令
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            retHandler(value);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            integerHandler(value);
            break;
        case KOOPA_RVT_BINARY:
            // 访问二元运算
            binaryHander(value);
            break;
        case KOOPA_RVT_ALLOC:
            allocHander(value);
            break;
        case KOOPA_RVT_LOAD:
            loadHander(value);
            break;
        case KOOPA_RVT_STORE:
            storeHander(value);
            break;

        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

void AsmGenerator::retHandler(const koopa_raw_value_t &value)
{

    koopa_raw_return_t inst = value->kind.data.ret;
    koopa_raw_value_kind_t kind = inst.value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_INTEGER:
            fos << "\tli a0, " << kind.data.integer.value << "\n";
            break;
        default:
            fos << "\tlw a0," << currentFunciton->mapAllocMem[inst.value] <<"(fp)\n";
            //assert(false);
    }
    fos << "\tmv sp, fp" << "\n";
    fos << "\tret\n";

}

void AsmGenerator::integerHandler(const koopa_raw_value_t &value)
{
    return;
}

void AsmGenerator::binaryHander(const koopa_raw_value_t &value){

    koopa_raw_binary_t inst = value->kind.data.binary;
    std::string leftReg, rightReg, expReg;
    koopa_raw_value_t leftValue = inst.lhs;
    koopa_raw_value_t rightValue = inst.rhs;
    // 分配临时寄存器
    expReg = currentFunciton->allocReg();
    int dst = currentFunciton->allocMem(value);
    
    leftReg = currentFunciton->allocReg();
    switch (leftValue->kind.tag) {
        case KOOPA_RVT_INTEGER:
            fos << "\tli " + leftReg + ", " << leftValue->kind.data.integer.value << "\n";
            break;
        default:
            fos << "\tlw " << leftReg << ", " << currentFunciton->mapAllocMem[leftValue] <<"(fp)\n";

    }

    rightReg = currentFunciton->allocReg();
    switch (rightValue->kind.tag) {
        case KOOPA_RVT_INTEGER:
            fos << "\tli " + rightReg + ", " << rightValue->kind.data.integer.value << "\n";
            break;
        default:
            fos << "\tlw " << rightReg << ", " << currentFunciton->mapAllocMem[rightValue] <<"(fp)\n";

    }

    // conserve reg
    std::string tempReg = currentFunciton->allocReg();

    switch (inst.op) {
        /// Not equal to.
        case KOOPA_RBO_NOT_EQ:
            fos << "\txor " + tempReg + ", " << leftReg << ", " << rightReg << "\n";
            fos << "\tsnez " + expReg + ", " << tempReg << "\n";
            break;

            /// Equal to.
        case KOOPA_RBO_EQ:
            fos << "\txor " + tempReg + ", " << leftReg << ", " << rightReg << "\n";
            fos << "\tseqz " + expReg + ", " << tempReg << "\n";
            break;

            /// Greater than.
        case KOOPA_RBO_GT:
            fos << "\tsgt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Less than.
        case KOOPA_RBO_LT:
            fos << "\tslt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Greater than or equal to.
        case KOOPA_RBO_GE:
            fos << "\tslt " + expReg + ", " << rightReg << ", " << leftReg << "\n";
            break;

            /// Less than or equal to.
        case KOOPA_RBO_LE:
            fos << "\tsgt " + expReg + ", " << rightReg << ", " << leftReg << "\n";
            break;

            /// Addition.
        case KOOPA_RBO_ADD:
            fos << "\tadd " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Subtraction.
        case KOOPA_RBO_SUB:
            fos << "\tsub " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Multiplication.
        case KOOPA_RBO_MUL:
            fos << "\tmul " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Division.
        case KOOPA_RBO_DIV:
            fos << "\tdiv " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Modulo.
        case KOOPA_RBO_MOD:
            fos << "\trem " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise AND.
        case KOOPA_RBO_AND:
            fos << "\tand " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise OR.
        case KOOPA_RBO_OR:
            fos << "\tor " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise XOR.
        case KOOPA_RBO_XOR:
            fos << "\txor " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift left logical.
        case KOOPA_RBO_SHL:
            fos << "\tsll " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift right logical.
        case KOOPA_RBO_SHR:
            fos << "\tsrl " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift right arithmetic.
        case KOOPA_RBO_SAR:
            fos << "\tsra " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;
    }
    currentFunciton->restoreReg(tempReg);
    currentFunciton->restoreReg(leftReg);
    currentFunciton->restoreReg(rightReg);
    fos << "\tsw " << expReg << ", " << dst <<"(fp)\n";
    currentFunciton->restoreReg(expReg);


    //if(leftValue->kind.tag != KOOPA_RVT_INTEGER) currentFunciton->restoreReg(leftReg);
    //if(rightValue->kind.tag != KOOPA_RVT_INTEGER) currentFunciton->restoreReg(rightReg);

    return;
}

void AsmGenerator::allocHander(const koopa_raw_value_t &value)
{
    currentFunciton->allocMem(value);
}

void AsmGenerator::loadHander(const koopa_raw_value_t &value)
{
    koopa_raw_load_t inst = value->kind.data.load;
    int src = currentFunciton->mapAllocMem[inst.src];
    int dst = currentFunciton->allocMem(value);
    std::string tempReg = currentFunciton->allocReg();
    switch(inst.src->kind.tag){
        case KOOPA_RVT_ALLOC:
            fos << "\tlw " << tempReg << ", " << src <<"(fp)\n";
            break;
        default:
            fos << "\tlw " << tempReg << ", " << src <<"(fp)\n";
            fos << "\tlw " << tempReg << ", 0(" << tempReg << ")\n";
    }

    fos << "\tsw " << tempReg << ", " << dst <<"(fp)\n";
    currentFunciton->restoreReg(tempReg);
}

void AsmGenerator::storeHander(const koopa_raw_value_t &value)
{
    koopa_raw_store_t inst = value->kind.data.store;
    koopa_raw_value_t src = inst.value;
    koopa_raw_value_t dst = inst.dest;
    std::string stempReg = currentFunciton->allocReg();
    std::string dtempReg = currentFunciton->allocReg();

    if(src->kind.tag == KOOPA_RVT_INTEGER){
        fos << "\tli " << stempReg << ", " << src->kind.data.integer.value << "\n";
    }
    else{
        fos << "\tlw " << stempReg << ", " << currentFunciton->mapAllocMem[src] <<"(fp)\n";
    }

    switch(dst->kind.tag){
        case KOOPA_RVT_ALLOC:
            fos << "\tsw " << stempReg << ", " << currentFunciton->mapAllocMem[dst] <<"(fp)\n";
            break;
        default:
            fos << "\tlw " << dtempReg << ", " << currentFunciton->mapAllocMem[dst] <<"(fp)\n";
            fos << "\tsw " << stempReg << ", 0(" << dtempReg << ")\n";

    }

    currentFunciton->restoreReg(stempReg);
    currentFunciton->restoreReg(dtempReg);
}


Function::Function():sp(0)
{
    initRegSet();
}

Function::Function(std::string namet):name(namet),sp(0)
{
    initRegSet();
}

void Function::initRegSet()
{
    setUnallocReg.insert({"t0","t1","t2","t3","t4","t5","t6"});
}


std::string Function::allocReg(const koopa_raw_value_t &value)
{
    if(setUnallocReg.empty()) assert(false);

    if(mapAllocReg.find(value)!=mapAllocReg.end()){
        return mapAllocReg[value];
    }
    else{
        mapAllocReg[value] = *setUnallocReg.begin();
        // if s0-s11, callee saved.
        setUnallocReg.erase(setUnallocReg.begin());
        return mapAllocReg[value];
    }
}

// 临时，只用一次
std::string Function::allocReg()
{
    if(setUnallocReg.empty()) assert(false);
    auto temp = *setUnallocReg.begin();
    setUnallocReg.erase(setUnallocReg.begin());
    return temp;
}

void Function::restoreReg(const std::string &reg)
{
    setUnallocReg.insert(reg);
}

int Function::allocMem(const koopa_raw_value_t &value)
{
    sp-=4;
    return (mapAllocMem[value] = sp);
}