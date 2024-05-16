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
#include <deque>

class SymbolInfo{
public:
    // 记录这个ident是定义的第几个名字
    static std::unordered_map<std::string, int> mapNameIndex;

    SymbolInfo(std::string id, std::string typet, bool isConstt, int constNumt);

    SymbolInfo(std::string id, std::string typet, bool isConstt);

    std::string name;
    std::string type;
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
    // BRANCH/OR/AND, then/else/end
    BlockInfo(std::string position);
    void generateIR(std::ostream &os);
public:
    bool finish;
    std::string name;
    static std::unordered_map<std::string , int> mapBlockIndex;
};


#endif //COMPILER_AUXILIARY_H
