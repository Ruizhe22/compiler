//
// Created by ruizhe on 24-5-16.
//
//
// Created by ruizhe on 24-4-13.
//

#include "ast.h"
#include "auxiliary.h"
#include <fstream>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <deque>

void BaseAST::generateIR(std::ostream &os) { return; }

void BaseAST::spreadSymbolTable() { return; }

BaseAST::~BaseAST() = default;

std::string BaseAST::getSymbolName(std::string ident) {
    return symbolTable[ident]->name;
}

CompUnitAST::CompUnitAST(BaseAST *f) : globalDefList(f) {}

void CompUnitAST::generateIR(std::ostream &os) {
    os << "decl @getint(): i32\n";
    os << "decl @getch(): i32\n";
    os << "decl @getarray(*i32): i32\n";
    os << "decl @putint(i32)\n";
    os << "decl @putch(i32)\n";
    os << "decl @putarray(i32, *i32)\n";
    os << "decl @starttime()\n";
    os << "decl @stoptime()\n";
    os << "\n";
    symbolTable["@getint"] = std::make_shared<SymbolInfo>("@getint", "i32");
    symbolTable["@getch"] = std::make_shared<SymbolInfo>("@getch", "i32");
    symbolTable["@getarray"] = std::make_shared<SymbolInfo>("@getarray", "i32");
    symbolTable["@putint"] = std::make_shared<SymbolInfo>("@putint", "void");
    symbolTable["@putch"] = std::make_shared<SymbolInfo>("@putch", "void");
    symbolTable["@putarray"] = std::make_shared<SymbolInfo>("@putarray", "void");
    symbolTable["@starttime"] = std::make_shared<SymbolInfo>("@starttime", "void");
    symbolTable["@stoptime"] = std::make_shared<SymbolInfo>("@stoptime", "void");
    globalDefList->symbolTable = symbolTable;
    globalDefList->spreadSymbolTable();
    globalDefList->generateIR(os);
}

GlobalDefListAST1::GlobalDefListAST1(BaseAST *d, BaseAST *l) : globalDef(d),globalDefList(l) {}
void GlobalDefListAST1::generateIR(std::ostream &os) {
    globalDef->generateIR(os);
    globalDefList->generateIR(os);
}
void GlobalDefListAST1::spreadSymbolTable(){
    globalDef->symbolTable = symbolTable;
    globalDef->spreadSymbolTable();
    symbolTable = globalDef->symbolTable;
    globalDefList->symbolTable = symbolTable;
    globalDefList->spreadSymbolTable();
}

GlobalDefAST::GlobalDefAST(BaseAST *d, bool b):def(d),isDecl(b){}
void GlobalDefAST::generateIR(std::ostream &os){
    def->generateIR(os);
}
void GlobalDefAST::spreadSymbolTable(){
    // add func name to symbol table
    if (!isDecl){
        auto func = std::dynamic_pointer_cast<FuncDefAST>(def);
        symbolTable[func->name] = std::make_shared<SymbolInfo>(func->name, func->type);
    }
    def->symbolTable = symbolTable;
    def->spreadSymbolTable();
    if (isDecl){
        symbolTable = def->symbolTable;
    }
}

ExpBaseAST::ExpBaseAST(bool fresh) : allocNewName(fresh) {}


FuncDefAST::FuncDefAST(std::string *ft, std::string *id, BaseAST *p, BaseAST *b) : type(*ft), name("@" + *id), funcFParamList(p), block(b) {}
void FuncDefAST::generateIR(std::ostream &os) {
    ExpBaseAST::expNum = 0;
    BlockInfo::mapBlockIndex.clear();
    currentFunction = std::make_shared<FunctionInfo>(name, type);
    os << "fun " << name << "(";
    //生成参数列表
    if (funcFParamList){
        std::dynamic_pointer_cast<FuncFParamListAST>(funcFParamList)->generateParamIR(os);
    }
    os<< ")";
    if (type == "i32"){
        os << ": i32 ";
    }
    os << " {\n";
    currentBlock = std::make_shared<BlockInfo>("entry");
    currentBlock->generateIR(os);
    // 进入entry，首先生成parameters分配空间
    if (funcFParamList) { funcFParamList->generateIR(os); }
    block->generateIR(os);

    if (!currentBlock->finish) {
        if (type == "i32"){
            os << "\tret 0\n";
        }
        else{
            os << "\tret\n";
        }
    }
    currentFunction = nullptr;
    os << "}\n\n";

}

void FuncDefAST::spreadSymbolTable() {
    if (funcFParamList) {
        funcFParamList->symbolTable = symbolTable;
        funcFParamList->spreadSymbolTable();
        symbolTable = funcFParamList->symbolTable;
    }
    block->symbolTable = symbolTable;
    block->spreadSymbolTable();
}

FuncFParamListAST::FuncFParamListAST(BaseAST *p, BaseAST *list):funcFParam(p),funcFParamList(list){}
void FuncFParamListAST::generateParamIR(std::ostream &os){
    std::dynamic_pointer_cast<FuncFParamAST>(funcFParam)->generateParamIR(os);
    if (funcFParamList) {
        os << ", ";
        std::dynamic_pointer_cast<FuncFParamListAST>(funcFParamList)->generateParamIR(os);
    }
}
void FuncFParamListAST::generateIR(std::ostream &os){
    funcFParam->generateIR(os);
    if (funcFParamList) { funcFParamList->generateIR(os); }
}
void FuncFParamListAST::spreadSymbolTable(){
    funcFParam->symbolTable = symbolTable;
    funcFParam->spreadSymbolTable();
    symbolTable = funcFParam->symbolTable;
    if (funcFParamList) {
        funcFParamList->symbolTable = symbolTable;
        funcFParamList->spreadSymbolTable();
        symbolTable = funcFParamList->symbolTable;
    }
}

FuncFParamAST::FuncFParamAST(std::string *t, std::string *id):type(*t),name("@" + *id){}
void FuncFParamAST::generateParamIR(std::ostream &os){
    //getSymbolName(name);
    os << getSymbolName(name) << ": " << type;
}
void FuncFParamAST::generateIR(std::ostream &os){
    // %x = alloc i32
    // store @x, %x
    std::string symbolName = getSymbolName(name);
    std::string tempName = "%" + symbolName.substr(1);
    symbolTable[name]->name = tempName;
    os << "\t" << tempName << " = alloc " << type << "\n";
    os << "\tstore " << symbolName << ", " << tempName << "\n";
}
void FuncFParamAST::spreadSymbolTable(){
    symbolTable[name] = std::make_shared<SymbolInfo>(name, type, false);
    //std::cout << "###### " << symbolTable.size() << symbolTable.contains(name) << symbolTable.begin()->first << "\n";
}

ExtendStmtAST::ExtendStmtAST(BaseAST *ast) : stmt(ast) {}

void ExtendStmtAST::generateIR(std::ostream &os) {
    stmt->generateIR(os);
}

void ExtendStmtAST::spreadSymbolTable() {
    stmt->symbolTable = symbolTable;
    stmt->spreadSymbolTable();
}

MatchStmtAST1::MatchStmtAST1(BaseAST *ast1, BaseAST *ast2, BaseAST *ast3) : exp(ast1), matchStmt1(ast2),
                                                                            matchStmt2(ast3) {}

void MatchStmtAST1::generateIR(std::ostream &os) {
    //if(currentBlock->finish) return;
    std::shared_ptr<BlockInfo> thenBlock = std::make_shared<BlockInfo>("then");
    std::shared_ptr<BlockInfo> elseBlock = std::make_shared<BlockInfo>("else");
    std::shared_ptr<BlockInfo> endBlock = std::make_shared<BlockInfo>("end");
    if(exp->isNum){
        os << "\t" << "br " << exp->num << ", " << thenBlock->name << ", " << elseBlock->name <<"\n";
    }
    else{
        exp->generateIR(os);
        os << "\t" << "br " << exp->name << ", " << thenBlock->name << ", " << elseBlock->name <<"\n";
    }
    thenBlock->generateIR(os);
    currentBlock = thenBlock;
    matchStmt1->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << endBlock->name <<"\n";
    currentBlock->finish = true;
    elseBlock->generateIR(os);
    currentBlock = elseBlock;
    matchStmt2->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << endBlock->name <<"\n";
    currentBlock->finish = true;
    endBlock->generateIR(os);
    currentBlock = endBlock;
}

void MatchStmtAST1::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    matchStmt1->symbolTable = symbolTable;
    matchStmt1->spreadSymbolTable();
    matchStmt2->symbolTable = symbolTable;
    matchStmt2->spreadSymbolTable();
}

MatchStmtAST2::MatchStmtAST2(BaseAST *ast) : stmt(ast) {}

void MatchStmtAST2::generateIR(std::ostream &os) {
    stmt->generateIR(os);
}

void MatchStmtAST2::spreadSymbolTable() {
    stmt->symbolTable = symbolTable;
    stmt->spreadSymbolTable();
}

OpenStmtAST1::OpenStmtAST1(BaseAST *ast1, BaseAST *ast2) : exp(ast1), extendStmt(ast2) {}

void OpenStmtAST1::generateIR(std::ostream &os) {
    //if(currentBlock->finish) return;
    std::shared_ptr<BlockInfo> thenBlock = std::make_shared<BlockInfo>("then");
    std::shared_ptr<BlockInfo> endBlock = std::make_shared<BlockInfo>("end");
    if(exp->isNum){
        os << "\t" << "br " << exp->num << ", " << thenBlock->name << ", " << endBlock->name <<"\n";
    }
    else{
        exp->generateIR(os);
        os << "\t" << "br " << exp->name << ", " << thenBlock->name << ", " << endBlock->name <<"\n";
    }
    thenBlock->generateIR(os);
    currentBlock = thenBlock;
    extendStmt->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << endBlock->name <<"\n";
    currentBlock->finish = true;
    endBlock->generateIR(os);
    currentBlock = endBlock;
}

void OpenStmtAST1::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    extendStmt->symbolTable = symbolTable;
    extendStmt->spreadSymbolTable();
}

OpenStmtAST2::OpenStmtAST2(BaseAST *ast1, BaseAST *ast2, BaseAST *ast3) : exp(ast1), matchStmt(ast2), openStmt(ast3) {}

void OpenStmtAST2::generateIR(std::ostream &os) {
    //if(currentBlock->finish) return;
    std::shared_ptr<BlockInfo> thenBlock = std::make_shared<BlockInfo>("then");
    std::shared_ptr<BlockInfo> elseBlock = std::make_shared<BlockInfo>("else");
    std::shared_ptr<BlockInfo> endBlock = std::make_shared<BlockInfo>("end");
    if(exp->isNum){
        os << "\t" << "br " << exp->num << ", " << thenBlock->name << ", " << elseBlock->name <<"\n";
    }
    else{
        exp->generateIR(os);
        os << "\t" << "br " << exp->name << ", " << thenBlock->name << ", " << elseBlock->name <<"\n";
    }
    thenBlock->generateIR(os);
    currentBlock = thenBlock;
    matchStmt->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << endBlock->name <<"\n";
    currentBlock->finish = true;
    elseBlock->generateIR(os);
    currentBlock = elseBlock;
    openStmt->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << endBlock->name <<"\n";
    currentBlock->finish = true;
    endBlock->generateIR(os);
    currentBlock = endBlock;
}

void OpenStmtAST2::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    matchStmt->symbolTable = symbolTable;
    matchStmt->spreadSymbolTable();
    openStmt->symbolTable = symbolTable;
    openStmt->spreadSymbolTable();
}

StmtAST1::StmtAST1(BaseAST *ast) : exp(ast) {}

StmtAST1::StmtAST1() = default;

void StmtAST1::generateIR(std::ostream &os) {
    if (exp){
        if (exp->isNum) {
            os << "\tret " << exp->num << "\n";
        } else {
            exp->generateIR(os);
            os << "\tret " << exp->name << "\n";
        }
    }
    else {
        if (currentFunction->type == "void"){ os << "\tret\n"; }
        else{ os << "\tret 0\n"; }
    }
    currentFunction->isReturn = true;
    currentBlock->finish = true;
}

void StmtAST1::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        isNum = true;
    }
    num = exp->num;
}


StmtAST2::StmtAST2(std::string *id, BaseAST *ast) : exp(ast) {
    name = "@" + *id;
}

void StmtAST2::generateIR(std::ostream &os) {
    if (exp->isNum) {
        os << "\tstore " << exp->num << ", " << getSymbolName(name) << "\n";
    } else {
        exp->generateIR(os);
        os << "\tstore " << exp->name << ", " << getSymbolName(name) << "\n";
    }
}

void StmtAST2::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
}

StmtAST3::StmtAST3(BaseAST *ast) : exp(ast) {}

StmtAST3::StmtAST3() = default;

void StmtAST3::generateIR(std::ostream &os) {
    if (exp && !exp->isNum) {
        exp->generateIR(os);
    }
}

void StmtAST3::spreadSymbolTable() {
    if (exp) {
        exp->symbolTable = symbolTable;
        exp->spreadSymbolTable();
    }
}


StmtAST4::StmtAST4(BaseAST *ast) : block(ast) {}

void StmtAST4::generateIR(std::ostream &os) {
    block->generateIR(os);
}

void StmtAST4::spreadSymbolTable() {
    block->symbolTable = symbolTable;
    block->spreadSymbolTable();
}

StmtAST5::StmtAST5(BaseAST *ast1, BaseAST *ast2):exp(ast1),extendStmt(ast2){}

void StmtAST5::generateIR(std::ostream &os){
    std::shared_ptr<BlockInfo> beginBlock = std::make_shared<BlockInfo>("begin");
    std::shared_ptr<BlockInfo> bodyBlock = std::make_shared<BlockInfo>("body");
    std::shared_ptr<BlockInfo> endBlock = std::make_shared<BlockInfo>("end");
    LoopInfo::pushLoopInfo(beginBlock, bodyBlock, endBlock);
    os << "\t" << "jump " << beginBlock->name <<"\n";
    beginBlock->generateIR(os);
    currentBlock = beginBlock;
    if(exp->isNum){
        os << "\t" << "br " << exp->num << ", " << bodyBlock->name << ", " << endBlock->name <<"\n";
    }
    else{
        exp->generateIR(os);
        os << "\t" << "br " << exp->name << ", " << bodyBlock->name << ", " << endBlock->name <<"\n";
    }
    bodyBlock->generateIR(os);
    currentBlock = beginBlock;
    extendStmt->generateIR(os);
    if(!currentBlock->finish) os << "\t" << "jump " << beginBlock->name <<"\n";
    currentBlock->finish = true;
    LoopInfo::popLoopInfo();
    endBlock->generateIR(os);
    currentBlock = endBlock;
}

void StmtAST5::spreadSymbolTable(){
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    extendStmt->symbolTable = symbolTable;
    extendStmt->spreadSymbolTable();
}

void StmtAST6::generateIR(std::ostream &os){
    os << "\t" << "jump " << LoopInfo::topLoopInfo().endBlock->name <<"\n";
    currentBlock->finish = true;
    // must not pop loopinfo here
}

void StmtAST7::generateIR(std::ostream &os){
    os << "\t" << "jump " << LoopInfo::topLoopInfo().beginBlock->name <<"\n";
    currentBlock->finish = true;
}

BlockAST::BlockAST(BaseAST *ast) : blockItemList(ast) {}

void BlockAST::generateIR(std::ostream &os) {
    //os << name <<":\n";
    blockItemList->generateIR(os);
}

void BlockAST::spreadSymbolTable() {
    //name = "%block_" + std::to_string(blockNum++);
    blockItemList->symbolTable = symbolTable;
    blockItemList->spreadSymbolTable();
    // 可以传回也可以不传回

}

BlockItemAST::BlockItemAST(BaseAST *itemt, bool isdecl) : item(itemt), isDecl(isdecl) {}

void BlockItemAST::spreadSymbolTable() {
    item->symbolTable = symbolTable;
    item->spreadSymbolTable();
    if (isDecl) { symbolTable = item->symbolTable; }
}

void BlockItemAST::generateIR(std::ostream &os) {
    item->generateIR(os);
}


DeclAST::DeclAST(BaseAST *declt, bool isconst) : decl(declt) {
    isNum = isconst;
}

void DeclAST::spreadSymbolTable() {
    // 先传下去 decl语句之前的符号表
    decl->symbolTable = symbolTable;
    decl->spreadSymbolTable();
    // 再传回 decl语句起作用之后的符号表
    symbolTable = decl->symbolTable;
}

void DeclAST::generateIR(std::ostream &os) {
    decl->generateIR(os);
}

ConstExpAST::ConstExpAST(BaseAST *expt) : exp(expt) {
    // without error detection
    isNum = exp->isNum;
    num = exp->num;
    name = exp->name;
}

void ConstExpAST::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        num = exp->num;
        isNum = true;
    }
}

ConstInitValAST::ConstInitValAST(BaseAST *constexp) : constExp(constexp) {
    isNum = constExp->isNum;
    num = constExp->num;
    name = constExp->name;
}

void ConstInitValAST::spreadSymbolTable() {
    constExp->symbolTable = symbolTable;
    constExp->spreadSymbolTable();
    if (constExp->isNum) {
        num = constExp->num;
        isNum = true;
    }
}

ConstDefAST::ConstDefAST(std::string *id, BaseAST *val) : constInitVal(val) {
    name = "@" + *id;
    num = constInitVal->num;
    isNum = constInitVal->isNum;
}

void ConstDefAST::spreadSymbolTable() {
    constInitVal->symbolTable = symbolTable;
    constInitVal->spreadSymbolTable();
    num = constInitVal->num;
    isNum = true;
    symbolTable[name] = std::make_shared<SymbolInfo>(name, type, true, num);
}


ConstDefListAST::ConstDefListAST(BaseAST *def) : defList(1, std::shared_ptr<BaseAST>(def)) {};

ConstDefListAST::ConstDefListAST(BaseAST *list, BaseAST *def) : defList(((ConstDefListAST *) list)->defList) {
    defList.push_back(std::shared_ptr<ConstDefAST>((ConstDefAST *) def));
}


ConstDeclAST::ConstDeclAST(std::string *btype, BaseAST *list) : type(*btype),
                                                                constDefList(((ConstDefListAST *) list)->defList) {}

void ConstDeclAST::spreadSymbolTable() {
    // 先传下去 decl语句之前的符号表 对于每个新的符号
    for (const auto &cd: constDefList) {
        std::dynamic_pointer_cast<ConstDefAST>(cd)->type = type;
        cd->symbolTable = symbolTable;
        cd->spreadSymbolTable();
        symbolTable = cd->symbolTable;
    }
}

InitValAST::InitValAST(BaseAST *expt) : exp(expt) {
    name = exp->name;
}

VarDeclAST::VarDeclAST(std::string *btype, BaseAST *list) :type(*btype),varDefList(((VarDefListAST *) list)->defList) {}
void VarDeclAST::generateIR(std::ostream &os) {
    for (const auto &vd: varDefList) {
        vd->generateIR(os);
    }
}
void VarDeclAST::spreadSymbolTable() {
    // 先传下去 decl语句之前的符号表 对于每个新的符号
    for (const auto &vd: varDefList) {
        std::dynamic_pointer_cast<VarDefAST>(vd)->type = type;
        vd->symbolTable = symbolTable;
        vd->spreadSymbolTable();
        symbolTable = vd->symbolTable;
    }
}

VarDefAST::VarDefAST(std::string *id) : isInit(false) {
    name = "@" + *id;
    isNum = false;
    num = 0;
}
VarDefAST::VarDefAST(std::string *id, BaseAST *init) : initVal(init), isInit(true) {
    name = "@" + *id;
    isNum = false;
}
void VarDefAST::spreadSymbolTable() {
    if (isInit) {
        initVal->symbolTable = symbolTable;
        initVal->spreadSymbolTable();
        num = initVal->num;
    }
    //std::cout<< "######" << name << " ###" << num << std::endl;
    symbolTable[name] = std::make_shared<SymbolInfo>(name, type, false, num);
}
void VarDefAST::generateIR(std::ostream &os) {
    if (currentFunction) {
        os << "\t" << getSymbolName(name) << " = alloc " << type << "\n";
        if (isInit) {
            if (initVal->isNum) {
                os << "\tstore " << initVal->num << ", " << getSymbolName(name) << "\n";
            } else {
                initVal->generateIR(os);
                os << "\tstore " << initVal->name << ", " << getSymbolName(name) << "\n";
            }
        }
    }
    else{
        if (isInit){
            os << "global " << getSymbolName(name) << " = alloc " << type << ", " << num << "\n";
        }
        else{
            os << "global " << getSymbolName(name) << " = alloc " << type << ", zeroinit\n";
        }
    }
}


VarDefListAST::VarDefListAST(BaseAST *def) : defList(1, std::shared_ptr<BaseAST>(def)) {};
VarDefListAST::VarDefListAST(BaseAST *list, BaseAST *def) : defList(((VarDefListAST *) list)->defList) {
    defList.push_back(std::shared_ptr<BaseAST>(def));
}

BlockItemListAST::BlockItemListAST() = default;

BlockItemListAST::BlockItemListAST(BaseAST *blockItemsTemp, BaseAST *blockItemTemp) : itemList(
        ((BlockItemListAST *) blockItemsTemp)->itemList) {
    auto temp = (BlockItemAST *) blockItemTemp;
    itemList.push_back(temp->item);
}

void BlockItemListAST::generateIR(std::ostream &os) {
    for (const auto &item: itemList) {
        if (currentBlock->finish) {
            continue;
        }
        item->generateIR(os);
    }
}

void BlockItemListAST::spreadSymbolTable() {
    for (const auto &blockItem: itemList) {
        blockItem->symbolTable = symbolTable;
        blockItem->spreadSymbolTable();
        symbolTable = blockItem->symbolTable;
    }
}

void InitValAST::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = exp->name;
    }
    num = exp->num;
}

void InitValAST::generateIR(std::ostream &os) {
    exp->generateIR(os);
}

ExpAST::ExpAST(BaseAST *ast) : ExpBaseAST(false), lOrExp(ast) {
    name = lOrExp->name;
}

void ExpAST::generateIR(std::ostream &os) {
    lOrExp->generateIR(os);
}

void ExpAST::spreadSymbolTable() {
    lOrExp->symbolTable = symbolTable;
    lOrExp->spreadSymbolTable();
    if (lOrExp->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = lOrExp->name;
    }
    num = lOrExp->num;
}

ExpBaseAST1::ExpBaseAST1(BaseAST *ast) : ExpBaseAST(false), delegate(ast) {
    name = delegate->name;
}

void ExpBaseAST1::generateIR(std::ostream &os) {
    delegate->generateIR(os);
}

void ExpBaseAST1::spreadSymbolTable() {
    delegate->symbolTable = symbolTable;
    delegate->spreadSymbolTable();
    if (delegate->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = delegate->name;
    }
    num = delegate->num;
}


ExpBaseAST2::ExpBaseAST2(BaseAST *ast1, BaseAST *ast2, const std::string &opt) : ExpBaseAST(true), left(ast1),
                                                                                 right(ast2), op(opt) {}

void ExpBaseAST2::generateIR(std::ostream &os) {
    if (left->isNum && right->isNum) {
        os << "\t" << name << " = " << op << " " << left->num << ", " << right->num << "\n";
    } else if (!left->isNum && right->isNum) {
        left->generateIR(os);
        os << "\t" << name << " = " << op << " " << left->name << ", " << right->num << "\n";
    } else if (left->isNum && !right->isNum) {
        right->generateIR(os);
        os << "\t" << name << " = " << op << " " << left->num << ", " << right->name << "\n";
    } else {
        left->generateIR(os);
        right->generateIR(os);
        os << "\t" << name << " = " << op << " " << left->name << ", " << right->name << "\n";
    }
}

void ExpBaseAST2::spreadSymbolTable() {
    left->symbolTable = symbolTable;
    left->spreadSymbolTable();
    right->symbolTable = symbolTable;
    right->spreadSymbolTable();
    if (left->isNum && right->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }
    if (op == "lt") { num = left->num < right->num; }
    if (op == "gt") { num = left->num > right->num; }
    if (op == "le") { num = left->num <= right->num; }
    if (op == "ge") { num = left->num >= right->num; }
    if (op == "add") { num = left->num + right->num; }
    if (op == "sub") { num = left->num - right->num; }
    if (op == "mul") { num = left->num * right->num; }
    if (op == "div") {
        if (right->num == 0) { num = 0; } else { num = left->num / right->num; }
    }
    if (op == "mod") {
        if (right->num == 0) { num = 0; } else { num = left->num % right->num; }
    }
    if (op == "eq") { num = left->num == right->num; }
    if (op == "ne") { num = left->num != right->num; }

}


LOrExpAST2::LOrExpAST2(BaseAST *ast1, BaseAST *ast2) : ExpBaseAST(true), left(ast1), right(ast2) {}

void LOrExpAST2::generateIR(std::ostream &os) {
    std::string temp1 = "%lorl" + name.substr(1);
    std::string temp2 = "%lorr" + name.substr(1);
    if (left->isNum && right->isNum) {
        os << "\t" << temp1 << " = ne 0," << left->num << "\n";
        os << "\t" << temp2 << " = ne 0," << right->num << "\n";
        os << "\t" << name << " =  or " << temp1 << ", " << temp2 << "\n";
    } else if (!left->isNum && right->isNum) {
        left->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->name << "\n";
        os << "\t" << temp2 << " = ne 0," << right->num << "\n";
        os << "\t" << name << " =  or " << temp1 << ", " << temp2 << "\n";
    } else if (left->isNum && !right->isNum) {
        right->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->num << "\n";
        os << "\t" << temp2 << " = ne 0," << right->name << "\n";
        os << "\t" << name << " =  or " << temp1 << ", " << temp2 << "\n";
    } else {
        left->generateIR(os);
        right->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->name << "\n";
        os << "\t" << temp2 << " = ne 0," << right->name << "\n";
        os << "\t" << name << " =  or " << temp1 << ", " << temp2 << "\n";
    }
}

void LOrExpAST2::spreadSymbolTable() {
    left->symbolTable = symbolTable;
    left->spreadSymbolTable();
    right->symbolTable = symbolTable;
    right->spreadSymbolTable();
    if (left->isNum && right->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }
    num = left->num || right->num;

}

LAndExpAST2::LAndExpAST2(BaseAST *ast1, BaseAST *ast2) : ExpBaseAST(true), left(ast1), right(ast2) {}

void LAndExpAST2::generateIR(std::ostream &os) {
    std::string temp1 = "%landl" + name.substr(1);
    std::string temp2 = "%landr" + name.substr(1);
    if (left->isNum && right->isNum) {
        os << "\t" << temp1 << " = ne 0," << left->num << "\n";
        os << "\t" << temp2 << " = ne 0," << right->num << "\n";
        os << "\t" << name << " =  and " << temp1 << ", " << temp2 << "\n";
    } else if (!left->isNum && right->isNum) {
        left->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->name << "\n";
        os << "\t" << temp2 << " = ne 0," << right->num << "\n";
        os << "\t" << name << " =  and " << temp1 << ", " << temp2 << "\n";
    } else if (left->isNum && !right->isNum) {
        right->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->num << "\n";
        os << "\t" << temp2 << " = ne 0," << right->name << "\n";
        os << "\t" << name << " =  and " << temp1 << ", " << temp2 << "\n";
    } else {
        left->generateIR(os);
        right->generateIR(os);
        os << "\t" << temp1 << " = ne 0," << left->name << "\n";
        os << "\t" << temp2 << " = ne 0," << right->name << "\n";
        os << "\t" << name << " =  and " << temp1 << ", " << temp2 << "\n";
    }
}

void LAndExpAST2::spreadSymbolTable() {
    left->symbolTable = symbolTable;
    left->spreadSymbolTable();
    right->symbolTable = symbolTable;
    right->spreadSymbolTable();
    if (left->isNum && right->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }
    num = left->num && right->num;

}


UnaryExpAST1::UnaryExpAST1(BaseAST *ast) : ExpBaseAST(false), primaryExp(ast) {
    name = primaryExp->name;
}

void UnaryExpAST1::generateIR(std::ostream &os) {
    primaryExp->generateIR(os);
}

void UnaryExpAST1::spreadSymbolTable() {
    primaryExp->symbolTable = symbolTable;
    primaryExp->spreadSymbolTable();
    if (primaryExp->isNum) {
        isNum = true;
    } else {
        isNum = false;
    }
    name = primaryExp->name;
    num = primaryExp->num;
}

UnaryExpAST2::UnaryExpAST2(std::string *opt, BaseAST *ast) : ExpBaseAST(true), unaryExp(ast), op(*opt) {}

void UnaryExpAST2::generateIR(std::ostream &os) {
    if (op == "not") op = "eq";
    if (unaryExp->isNum) {
        // no use
        if (op != "add") {
            os << "\t" << name << "= " << op << " 0, " << unaryExp->num << "\n";
        }
    } else {
        unaryExp->generateIR(os);
        if (op != "add") {
            os << "\t" << name << "= " << op << " 0, " << unaryExp->name << "\n";
        }
    }

}

void UnaryExpAST2::spreadSymbolTable() {
    unaryExp->symbolTable = symbolTable;
    unaryExp->spreadSymbolTable();
    if (unaryExp->isNum) {
        isNum = true;
    } else {
        if (op != "add") {
            name = "%" + std::to_string(++ExpBaseAST::expNum);
        } else {
            name = unaryExp->name;
        }
    }

    if (op == "add") {
        num = unaryExp->num;
    } else if (op == "sub") {
        num = -unaryExp->num;
    } else if (op == "not") {
        num = (0 == unaryExp->num);
    }

}

UnaryExpAST3::UnaryExpAST3(std::string *id, BaseAST *ast):ExpBaseAST(true), ident("@" + *id),funcRParamList(ast){}
void UnaryExpAST3::generateIR(std::ostream &os){
    if (funcRParamList) { funcRParamList->generateIR(os); }
    std::string func_name = getSymbolName(ident);
    os << "\t";
    if (!isVoid){
        os << name << " = ";
    }
    os << "call " << func_name << "(";
    if (funcRParamList) { std::dynamic_pointer_cast<FuncRParamListAST>(funcRParamList)->generateParam(os); }
    os << ")\n";
}
void UnaryExpAST3::spreadSymbolTable(){
    //std::cout << "#####" << ident <<std::endl;
    if (funcRParamList) {
        funcRParamList->symbolTable = symbolTable;
        funcRParamList->spreadSymbolTable();
    }
    isNum = false;
    if (symbolTable[ident]->type == "void"){
        isVoid = true;
    }
    else{
        //std::cout << "#####ok1" << ident <<std::endl;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
        //std::cout << "#####ok2" <<ident << std::endl;
        isVoid = false;
    }
}

FuncRParamListAST::FuncRParamListAST(BaseAST *ast1, BaseAST *ast2):exp(ast1),funcRParamList(ast2){}
void FuncRParamListAST::generateIR(std::ostream &os){
    if (!exp->isNum) { exp->generateIR(os); }
    if (funcRParamList) {funcRParamList->generateIR(os);}
}
void FuncRParamListAST::generateParam(std::ostream &os){
    if (exp->isNum){ os << exp->num; }
    else{ os << exp->name; }
    if (funcRParamList){
        os << ", ";
        std::dynamic_pointer_cast<FuncRParamListAST>(funcRParamList)->generateParam(os);
    }
}
void FuncRParamListAST::spreadSymbolTable(){
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (funcRParamList) {
        funcRParamList->symbolTable = symbolTable;
        funcRParamList->spreadSymbolTable();
    }
}

PrimaryExpAST1::PrimaryExpAST1(int n) : ExpBaseAST(false) {
    isNum = true;
    num = n;
}

void PrimaryExpAST1::spreadSymbolTable() {
    return;
}


PrimaryExpAST2::PrimaryExpAST2(BaseAST *ast) : ExpBaseAST(false), exp(ast) {
    if (exp->isNum) {
        isNum = true;
    }
    name = exp->name;
}
void PrimaryExpAST2::generateIR(std::ostream &os) {
    exp->generateIR(os);
}
void PrimaryExpAST2::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        isNum = true;
    }
    name = exp->name;
    num = exp->num;
}

PrimaryExpAST3::PrimaryExpAST3(std::string *id) : ident("@" + *id) {
    name = ident;
    isID = true;
}

void PrimaryExpAST3::spreadSymbolTable() {
    std::shared_ptr <SymbolInfo> symbol = symbolTable[ident];
    if (symbol->isNum) {
        isNum = true;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }
    num = symbol->num;
    return;
}
void PrimaryExpAST3::generateIR(std::ostream &os) {
    if (!isNum) {
        os << "\t" << name << " = load " << symbolTable[ident]->name << "\n";
    }
}


