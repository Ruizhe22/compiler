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

AsmGenerator::AsmGenerator(std::stringstream &ss, std::ostream &os):koopa(ss), ofs(os){}

void AsmGenerator::rawHandler(const koopa_raw_program_t &raw)
{
    // 执行一些其他的必要操作
    ofs << "\t.text\n"
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
        ofs << funcName << ":\n";
        mapFunction[funcName] = currentFunciton;
    }
    // 更新帧指针
    ofs << "\tmv fp, sp" << "\n";
    // 访问所有基本块
    visitRawSlice(func->bbs);
    //ofs << "\taddi sp, sp, " << currentFunciton->sp << "\n";
    ofs << oss.rdbuf();
}

// 访问基本块
void AsmGenerator::visitRawBlock(const koopa_raw_basic_block_t &bb)
{
    if(bb->name){
        std::string blockName = bb->name + 1;
        oss << blockName << ":\n";
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
            binaryHandler(value);
            break;
        case KOOPA_RVT_ALLOC:
            allocHandler(value);
            break;
        case KOOPA_RVT_LOAD:
            loadHandler(value);
            break;
        case KOOPA_RVT_STORE:
            storeHandler(value);
            break;
        case KOOPA_RVT_BRANCH:
            branchHandler(value);
            break;
        case KOOPA_RVT_JUMP:
            jumpHandler(value);
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
            oss << "\tli a0, " << kind.data.integer.value << "\n";
            break;
        default:
            oss << "\tlw a0," << currentFunciton->mapAllocMem[inst.value] <<"(fp)\n";
            //assert(false);
    }
    oss << "\tmv sp, fp" << "\n";
    oss << "\tret\n";

}

void AsmGenerator::integerHandler(const koopa_raw_value_t &value)
{
    return;
}

void AsmGenerator::binaryHandler(const koopa_raw_value_t &value){

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
            oss << "\tli " + leftReg + ", " << leftValue->kind.data.integer.value << "\n";
            break;
        default:
            oss << "\tlw " << leftReg << ", " << currentFunciton->mapAllocMem[leftValue] <<"(fp)\n";

    }

    rightReg = currentFunciton->allocReg();
    switch (rightValue->kind.tag) {
        case KOOPA_RVT_INTEGER:
            oss << "\tli " + rightReg + ", " << rightValue->kind.data.integer.value << "\n";
            break;
        default:
            oss << "\tlw " << rightReg << ", " << currentFunciton->mapAllocMem[rightValue] <<"(fp)\n";

    }

    // conserve reg
    std::string tempReg = currentFunciton->allocReg();

    switch (inst.op) {
        /// Not equal to.
        case KOOPA_RBO_NOT_EQ:
            oss << "\txor " + tempReg + ", " << leftReg << ", " << rightReg << "\n";
            oss << "\tsnez " + expReg + ", " << tempReg << "\n";
            break;

            /// Equal to.
        case KOOPA_RBO_EQ:
            oss << "\txor " + tempReg + ", " << leftReg << ", " << rightReg << "\n";
            oss << "\tseqz " + expReg + ", " << tempReg << "\n";
            break;

            /// Greater than.
        case KOOPA_RBO_GT:
            oss << "\tsgt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Less than.
        case KOOPA_RBO_LT:
            oss << "\tslt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Greater than or equal to.
        case KOOPA_RBO_GE:
            oss << "\tslt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            oss << "\tseqz " + expReg + ", " << expReg << "\n";
            break;

            /// Less than or equal to.
        case KOOPA_RBO_LE:
            oss << "\tsgt " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            oss << "\tseqz " + expReg + ", " << expReg << "\n";
            break;

            /// Addition.
        case KOOPA_RBO_ADD:
            oss << "\tadd " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Subtraction.
        case KOOPA_RBO_SUB:
            oss << "\tsub " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Multiplication.
        case KOOPA_RBO_MUL:
            oss << "\tmul " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Division.
        case KOOPA_RBO_DIV:
            oss << "\tdiv " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Modulo.
        case KOOPA_RBO_MOD:
            oss << "\trem " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise AND.
        case KOOPA_RBO_AND:
            oss << "\tand " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise OR.
        case KOOPA_RBO_OR:
            oss << "\tor " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Bitwise XOR.
        case KOOPA_RBO_XOR:
            oss << "\txor " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift left logical.
        case KOOPA_RBO_SHL:
            oss << "\tsll " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift right logical.
        case KOOPA_RBO_SHR:
            oss << "\tsrl " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;

            /// Shift right arithmetic.
        case KOOPA_RBO_SAR:
            oss << "\tsra " + expReg + ", " << leftReg << ", " << rightReg << "\n";
            break;
    }
    currentFunciton->restoreReg(tempReg);
    currentFunciton->restoreReg(leftReg);
    currentFunciton->restoreReg(rightReg);
    oss << "\tsw " << expReg << ", " << dst <<"(fp)\n";
    currentFunciton->restoreReg(expReg);


    //if(leftValue->kind.tag != KOOPA_RVT_INTEGER) currentFunciton->restoreReg(leftReg);
    //if(rightValue->kind.tag != KOOPA_RVT_INTEGER) currentFunciton->restoreReg(rightReg);

    return;
}

void AsmGenerator::allocHandler(const koopa_raw_value_t &value)
{
    currentFunciton->allocMem(value);
}

void AsmGenerator::loadHandler(const koopa_raw_value_t &value)
{
    koopa_raw_load_t inst = value->kind.data.load;
    int src = currentFunciton->mapAllocMem[inst.src];
    int dst = currentFunciton->allocMem(value);
    std::string tempReg = currentFunciton->allocReg();
    switch(inst.src->kind.tag){
        case KOOPA_RVT_ALLOC:
            oss << "\tlw " << tempReg << ", " << src <<"(fp)\n";
            break;
        default:
            oss << "\tlw " << tempReg << ", " << src <<"(fp)\n";
            oss << "\tlw " << tempReg << ", 0(" << tempReg << ")\n";
    }

    oss << "\tsw " << tempReg << ", " << dst <<"(fp)\n";
    currentFunciton->restoreReg(tempReg);
}

void AsmGenerator::storeHandler(const koopa_raw_value_t &value)
{
    koopa_raw_store_t inst = value->kind.data.store;
    koopa_raw_value_t src = inst.value;
    koopa_raw_value_t dst = inst.dest;
    std::string stempReg = currentFunciton->allocReg();
    std::string dtempReg = currentFunciton->allocReg();

    if(src->kind.tag == KOOPA_RVT_INTEGER){
        oss << "\tli " << stempReg << ", " << src->kind.data.integer.value << "\n";
    }
    else{
        oss << "\tlw " << stempReg << ", " << currentFunciton->mapAllocMem[src] <<"(fp)\n";
    }

    switch(dst->kind.tag){
        case KOOPA_RVT_ALLOC:
            oss << "\tsw " << stempReg << ", " << currentFunciton->mapAllocMem[dst] <<"(fp)\n";
            break;
        default:
            oss << "\tlw " << dtempReg << ", " << currentFunciton->mapAllocMem[dst] <<"(fp)\n";
            oss << "\tsw " << stempReg << ", 0(" << dtempReg << ")\n";

    }

    currentFunciton->restoreReg(stempReg);
    currentFunciton->restoreReg(dtempReg);
}

void AsmGenerator::branchHandler(const koopa_raw_value_t &value)
{
    koopa_raw_basic_block_t trueBlock = value->kind.data.branch.true_bb;
    koopa_raw_basic_block_t falseBlock = value->kind.data.branch.false_bb;
    std::string trueLabel = trueBlock->name + 1;
    std::string falseLabel = falseBlock->name + 1;
    koopa_raw_value_t cond = value->kind.data.branch.cond;
    std::string condReg = currentFunciton->allocReg();
    if(cond->kind.tag == KOOPA_RVT_INTEGER){
        oss << "\tli " << condReg << ", " << cond->kind.data.integer.value << "\n";
    }else{
        oss << "\tlw " << condReg << ", " << currentFunciton->mapAllocMem[cond] <<"(fp)\n";;
    }

    std::string label = AsmTempLabel::tempLabel();
    oss << "\tbnez " << condReg << ", " << label << "\n";
    oss << "\tj " << falseLabel << "\n";
    oss << label << ":\n";
    oss << "\tj " << trueLabel << "\n";
    currentFunciton->restoreReg(condReg);
}

void AsmGenerator::jumpHandler(const koopa_raw_value_t &value)
{
    koopa_raw_basic_block_t target = value->kind.data.jump.target;
    oss << "\tj " << target->name + 1 << "\n";
}


