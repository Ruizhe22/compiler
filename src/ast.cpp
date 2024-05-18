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

CompUnitAST::CompUnitAST(BaseAST *f) : funcDef(f) {}

void CompUnitAST::generateIR(std::ostream &os) {
    funcDef->symbolTable = symbolTable;
    funcDef->spreadSymbolTable();
    funcDef->generateIR(os);
}


ExpBaseAST::ExpBaseAST(bool fresh) : allocNewName(fresh) {}

FuncDefAST::FuncDefAST(BaseAST *ft, std::string *s, BaseAST *b) : funcType(ft), name("@" + *s), block(b) {
    // exp restart to count from 0 in a new func
    ExpBaseAST::expNum = 0;
}

void FuncDefAST::generateIR(std::ostream &os) {
    currentFunction = std::make_shared<FunctionInfo>(name);
    os << "fun " << name << "()";
    funcType->generateIR(os);
    os << " {\n";
    currentBlock = std::make_shared<BlockInfo>("entry");
    currentBlock->generateIR(os);
    block->generateIR(os);
    if (!currentBlock->finish) {
        os << "\tret 0\n";
    }
    currentFunction = nullptr;
    os << "}";

}

void FuncDefAST::spreadSymbolTable() {
    block->symbolTable = symbolTable;
    block->spreadSymbolTable();
}

FuncTypeAST::FuncTypeAST(const std::string &t) : type(t) {}

void FuncTypeAST::generateIR(std::ostream &os) {
    os << ":" << type;
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
    if (exp->isNum) {
        os << "\tret " << exp->num << "\n";
    } else {
        exp->generateIR(os);
        os << "\tret " << exp->name << "\n";
    }
    currentFunction->isReturn = true;
    currentBlock->finish = true;
}

void StmtAST1::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        isNum = true;
        num = exp->num;
    }
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
    if (isDecl) { symbolTable.merge(item->symbolTable); }
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
        cd->symbolTable = symbolTable;
        cd->spreadSymbolTable();
        symbolTable = cd->symbolTable;
    }
}

InitValAST::InitValAST(BaseAST *expt) : exp(expt) {
    name = exp->name;
}

void InitValAST::spreadSymbolTable() {
    exp->symbolTable = symbolTable;
    exp->spreadSymbolTable();
    if (exp->isNum) {
        num = exp->num;
        isNum = true;
    } else {
        isNum = false;
        name = exp->name;
    }
}

void InitValAST::generateIR(std::ostream &os) {
    exp->generateIR(os);
}

VarDefAST::VarDefAST(std::string *id) : isInit(false) {
    name = "@" + *id;
    isNum = false;
}

VarDefAST::VarDefAST(std::string *id, BaseAST *init) : initVal(init), isInit(true) {
    name = "@" + *id;
    isNum = false;
}

void VarDefAST::spreadSymbolTable() {
    if (isInit) {
        initVal->symbolTable = symbolTable;
        initVal->spreadSymbolTable();
    }

    symbolTable[name] = std::make_shared<SymbolInfo>(name, type, false);
}

void VarDefAST::generateIR(std::ostream &os) {
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


VarDefListAST::VarDefListAST(BaseAST *def) : defList(1, std::shared_ptr<BaseAST>(def)) {};

VarDefListAST::VarDefListAST(BaseAST *list, BaseAST *def) : defList(((VarDefListAST *) list)->defList) {
    defList.push_back(std::shared_ptr<BaseAST>(def));
}


VarDeclAST::VarDeclAST(std::string *btype, BaseAST *list) : varDefList(((VarDefListAST *) list)->defList) {
    type = *btype;
}

void VarDeclAST::generateIR(std::ostream &os) {
    for (const auto &vd: varDefList) {
        vd->generateIR(os);
    }
}

void VarDeclAST::spreadSymbolTable() {
    // 先传下去 decl语句之前的符号表 对于每个新的符号
    for (const auto &vd: varDefList) {
        vd->symbolTable = symbolTable;
        vd->spreadSymbolTable();
        symbolTable = vd->symbolTable;
    }
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
        num = lOrExp->num;
    } else {
        isNum = false;
        name = lOrExp->name;
    }
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
        num = delegate->num;
    } else {
        isNum = false;
        name = delegate->name;
    }
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
        if (op == "lt") { num = left->num < right->num; }
        if (op == "gt") { num = left->num > right->num; }
        if (op == "le") { num = left->num <= right->num; }
        if (op == "ge") { num = left->num >= right->num; }
        if (op == "add") { num = left->num + right->num; }
        if (op == "sub") { num = left->num - right->num; }
        if (op == "mul") { num = left->num * right->num; }
        if (op == "div") { num = left->num / right->num; }
        if (op == "mod") { num = left->num % right->num; }
        if (op == "eq") { num = left->num == right->num; }
        if (op == "ne") { num = left->num != right->num; }
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }

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
        num = left->num || right->num;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }

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
        num = left->num && right->num;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }

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
        num = primaryExp->num;
    } else {
        isNum = false;
    }
    name = primaryExp->name;

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
        if (op == "add") {
            num = unaryExp->num;
        } else if (op == "sub") {
            num = -unaryExp->num;
        } else if (op == "not") {
            num = (0 == unaryExp->num);
        }
    } else {
        if (op != "add") {
            name = "%" + std::to_string(++ExpBaseAST::expNum);
        } else {
            name = unaryExp->name;
        }
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
        num = exp->num;
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
        num = exp->num;
    }
    name = exp->name;
}

PrimaryExpAST3::PrimaryExpAST3(std::string *id) : ident("@" + *id) {
    name = ident;
    isID = true;
}

void PrimaryExpAST3::spreadSymbolTable() {
    std::shared_ptr <SymbolInfo> symbol = symbolTable[ident];
    if (symbol->isNum) {
        isNum = true;
        num = symbol->num;
    } else {
        isNum = false;
        name = "%" + std::to_string(++ExpBaseAST::expNum);
    }

    return;
}

void PrimaryExpAST3::generateIR(std::ostream &os) {
    if (!isNum) {
        os << "\t" << name << " = load " << symbolTable[ident]->name << "\n";
    }
}


