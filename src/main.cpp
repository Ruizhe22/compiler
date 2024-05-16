#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include "koopa.h"
#include "ast.h"
#include "asm_generator.h"
#include "auxiliary.h"
// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);
int ExpBaseAST::expNum = 0;
std::unordered_map<std::string, int> SymbolInfo::mapNameIndex;
std::unordered_map<std::string, int> BlockInfo::mapBlockIndex;
std::shared_ptr<FunctionInfo> BaseAST::currentFunction;
std::shared_ptr<BlockInfo> BaseAST::currentBlock;

int main(int argc, const char *argv[]) {
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    std::string mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);
    // 打开输出文件流
    std::ofstream fos(output);
    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    std::unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    // generate koopa ir
    if(mode == "-koopa"){
        ast->generateIR(fos);
    }
    else if(mode == "-riscv"){
        std::stringstream ss;
        ast->generateIR(ss);
        AsmGenerator riscvGenerator(ss, fos);
        riscvGenerator.generateRiscv();
    }

    return 0;
}
