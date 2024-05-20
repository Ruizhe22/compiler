//
// Created by ruizhe on 24-5-18.
//

#include "asm_auxiliary.h"
Function::Function(const koopa_raw_function_t &funct):func(funct),sp(-4)
{
    initRegSet();
}

void Function::initRegSet()
{
    setUnallocReg.insert({"t0","t1","t2","t3","t4","t5","t6"});
}

//分配寄存器需要显式释放
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

//只分配，不查找已分配
int Function::allocMem(const koopa_raw_value_t &value)
{
    sp-=4;
    return (mapAllocMem[value] = sp);
}

int Function::paramIndex(koopa_raw_value_t v){
    int i = 0;
    for(; i < func->params.len; ++i){
        if(func->params.buffer[i] == (void *)v)
            break;
    }
    return i;
}


AsmTempLabel AsmTempLabel::globalLabel;

std::string AsmTempLabel::label()
{
    return "lable_" + std::to_string(++tempLabelCount);
}
std::string AsmTempLabel::tempLabel()
{
    return globalLabel.label();
}
