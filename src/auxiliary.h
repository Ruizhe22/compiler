//
// Created by ruizhe on 24-5-16.
//

#ifndef COMPILER_AUXILIARY_H
#define COMPILER_AUXILIARY_H

#include <fstream>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <stack>

class SymbolInfo{
public:
    // 记录这个ident是定义的第几个名字
    static std::unordered_map<std::string, int> mapNameIndex;
    //for variable
    SymbolInfo(std::string id, std::string typet, bool isConstt, int constNumt);
    SymbolInfo(std::string id, std::string typet, bool isConstt);
    // for func
    SymbolInfo(std::string id, std::string typet);

    std::string name;
    std::string type;
    bool isFunc
    // isNum表示是不是const变量
    bool isNum;
    int num;

    std::string ident;
};


class FunctionInfo{
public:
    FunctionInfo(const std::string &namet);
    std::string name;
    bool isReturn;
};


// basic block in IR
class BlockInfo{
public:
    // then/else/end/begin(while)/body/entry
    BlockInfo(std::string position);
    void generateIR(std::ostream &os);
public:
    bool finish;
    std::string name;
    static std::unordered_map<std::string , int> mapBlockIndex;
};

// 单例模式
class LoopInfo{
public:
    static std::stack<LoopInfo> loopStack;
    static void pushLoopInfo(std::shared_ptr<BlockInfo> beginBlockt, std::shared_ptr<BlockInfo> bodyBlockt, std::shared_ptr<BlockInfo> endBlockt);
    static void popLoopInfo();
    static LoopInfo &topLoopInfo();
    std::shared_ptr<BlockInfo> beginBlock;
    std::shared_ptr<BlockInfo> bodyBlock;
    std::shared_ptr<BlockInfo> endBlock;
};


#endif //COMPILER_AUXILIARY_H
