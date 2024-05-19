//
// Created by ruizhe on 24-5-16.
//

#include "auxiliary.h"


SymbolInfo::SymbolInfo(std::string id, std::string typet, bool isConstt, int initNumt) : type(typet), isFunc(false), isNum(isConstt),
                                                                                          num(initNumt), ident(id) {
    if (mapNameIndex.contains(id)) {
        ++mapNameIndex[id];
    } else {
        mapNameIndex[id] = 0;
    }
    name = id + "_" + std::to_string(mapNameIndex[id]);
}

SymbolInfo::SymbolInfo(std::string id, std::string typet, bool isConstt) : type(typet), isFunc(false), isNum(isConstt),
                                                                            num(0), ident(id) {
    if (mapNameIndex.contains(id)) {
        ++mapNameIndex[id];
    } else {
        mapNameIndex[id] = 0;
    }
    name = id + "_" + std::to_string(mapNameIndex[id]);
}

SymbolInfo::SymbolInfo(std::string id, std::string typet) : type(typet), isFunc(true), isNum(false), ident(id){
    if (mapNameIndex.contains(id)) {
        ++mapNameIndex[id];
    } else {
        mapNameIndex[id] = 0;
    }
    name = id;
}


FunctionInfo::FunctionInfo(const std::string &namet, const std::string &typet): name(namet), type(typet), isReturn(false) {}


BlockInfo::BlockInfo(std::string position):finish(false){
    if (mapBlockIndex.contains(position)) {
        ++mapBlockIndex[position];
    } else {
        mapBlockIndex[position] = 0;
    }
    name = "%" + position + "_" + std::to_string(mapBlockIndex[position]);
}


void BlockInfo::generateIR(std::ostream &os){
    os << name << ":\n";
}

std::stack<LoopInfo> LoopInfo::loopStack;;
void LoopInfo::pushLoopInfo(std::shared_ptr<BlockInfo> beginBlockt, std::shared_ptr<BlockInfo> bodyBlockt, std::shared_ptr<BlockInfo> endBlockt) {
    LoopInfo loop;
    loop.beginBlock = beginBlockt;
    loop.bodyBlock = bodyBlockt;
    loop.endBlock = endBlockt;
    loopStack.push(loop);
}
void LoopInfo::popLoopInfo(){
    if (!loopStack.empty()) loopStack.pop();
}
LoopInfo &LoopInfo::topLoopInfo(){
    return loopStack.top();
}