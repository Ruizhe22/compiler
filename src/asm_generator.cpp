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
    std::string funcName = func->name + 1;
    fos << funcName << ":\n";
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
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            retHandler(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            integerHandler(kind.data.integer);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

void AsmGenerator::retHandler(const koopa_raw_return_t &inst)
{
    koopa_raw_value_kind_t kind = inst.value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_INTEGER:
            fos << "\tli a0, " << kind.data.integer.value << "\n";
            fos << "\tret\n";
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

void AsmGenerator::integerHandler(const koopa_raw_integer_t &inst)
{
    return;
}

