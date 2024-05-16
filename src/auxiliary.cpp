//
// Created by ruizhe on 24-5-16.
//

#include "auxiliary.h"


SymbolInfo::SymbolInfo(std::string id, std::string typet, bool isConstt, int constNumt) : type(typet), isNum(isConstt),
                                                                                          num(constNumt), ident(id) {
    if (mapNameIndex.contains(id)) {
        ++mapNameIndex[id];
    } else {
        mapNameIndex[id] = 0;
    }
    name = id + "_" + std::to_string(mapNameIndex[id]);
}

SymbolInfo::SymbolInfo(std::string id, std::string typet, bool isConstt) : type(typet), isNum(isConstt), ident(id) {
    if (mapNameIndex.contains(id)) {
        ++mapNameIndex[id];
    } else {
        mapNameIndex[id] = 0;
    }
    name = id + "_" + std::to_string(mapNameIndex[id]);
}


FunctionInfo::FunctionInfo(const std::string &namet) : name(namet), isReturn(false) {}


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
