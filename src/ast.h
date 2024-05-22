//
// Created by ruizhe on 24-4-13.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <fstream>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <deque>
#include "auxiliary.h"


// 所有 AST 的基类
class BaseAST {
public:
    virtual void generateIR(std::ostream &os);
    virtual void spreadSymbolTable();
    virtual ~BaseAST();
    std::string name;
    bool isNum = false;
    bool isID = false;
    int num;
    std::string type = "i32";
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> symbolTable;
    // if n
    static std::shared_ptr<FunctionInfo> currentFunction;
    // meaning basic block
    static std::shared_ptr<BlockInfo> currentBlock;

    std::string getSymbolName(std::string ident);

    void parseSymbolTable(){
        for (auto &pair : symbolTable) {
            std::cout << "####" << pair.first << "####" << pair.second->name << std::endl;
        }
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    CompUnitAST(BaseAST *f);
    void generateIR(std::ostream &os);

private:
    std::shared_ptr<BaseAST> globalDefList;

};

class GlobalDefAST : public BaseAST{
private:
    //FuncDefAST or DeclAST
    std::shared_ptr<BaseAST> def;
public:
    GlobalDefAST(BaseAST *d, bool b);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    bool isDecl;
};

class GlobalDefListAST1 : public BaseAST {
public:
    GlobalDefListAST1(BaseAST *d, BaseAST *l);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    std::shared_ptr<BaseAST> globalDef;
    std::shared_ptr<BaseAST> globalDefList;
};

class GlobalDefListAST2 : public BaseAST {
public:
    GlobalDefListAST2() = default;
};



class ExpBaseAST: public BaseAST {
public:
    //restore to 0 in funcDef construct
    static int expNum;
    // if not change expNum, child need to assign name in the body
    ExpBaseAST(bool fresh);
    bool allocNewName;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    FuncDefAST(std::string *ft, std::string *id, BaseAST *p, BaseAST *b);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    std::string type;
    std::string name;
private:
    std::shared_ptr<BaseAST> funcFParamList;
    std::shared_ptr<BaseAST> block;
};

class FuncFParamListAST : public BaseAST {
public:
    FuncFParamListAST(BaseAST *p, BaseAST *list);
    void generateIR(std::ostream &os);
    void generateParamIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> funcFParam;
    std::shared_ptr<BaseAST> funcFParamList;
};

class FuncFParamAST : public BaseAST {
public:
    FuncFParamAST(std::string *t, std::string *id);
    void generateIR(std::ostream &os);
    void generateParamIR(std::ostream &os);
    void spreadSymbolTable();
    std::string type;
    std::string name;
};

class FuncFParamAST2 : public BaseAST {
public:
    FuncFParamAST2(std::string *t, std::string *id, BaseAST *a);
    void generateIR(std::ostream &os);
    void generateParamIR(std::ostream &os);
    void spreadSymbolTable();
    std::string type;
    std::string name;
    BaseAST *arrayIndexList;
};

class ExtendStmtAST: public BaseAST {
public:
    ExtendStmtAST(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> stmt;
};

class MatchStmtAST1: public BaseAST {
public:
    MatchStmtAST1(BaseAST *ast1, BaseAST *ast2, BaseAST *ast3);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
    std::shared_ptr<BaseAST> matchStmt1;
    std::shared_ptr<BaseAST> matchStmt2;
};

class MatchStmtAST2: public BaseAST {
public:
    MatchStmtAST2(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> stmt;
};

class OpenStmtAST1: public BaseAST {
public:
    OpenStmtAST1(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();

private:
    std::shared_ptr<BaseAST> exp;
    std::shared_ptr<BaseAST> extendStmt;
};

class OpenStmtAST2: public BaseAST {
public:
    OpenStmtAST2(BaseAST *ast1, BaseAST *ast2, BaseAST *ast3);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
    std::shared_ptr<BaseAST> matchStmt;
    std::shared_ptr<BaseAST> openStmt;
};

class StmtAST1: public BaseAST {
public:
    StmtAST1(BaseAST *ast);
    StmtAST1();
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
};

class StmtAST2: public BaseAST {
public:
    StmtAST2(std::string *id, BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
};

class StmtAST3: public BaseAST {
public:
    StmtAST3(BaseAST *ast);
    StmtAST3();
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
};

class StmtAST4: public BaseAST {
public:
    StmtAST4(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> block;
};

class StmtAST5: public BaseAST {
public:
    StmtAST5(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
    std::shared_ptr<BaseAST> extendStmt;
};

class StmtAST6: public BaseAST {
public:
    void generateIR(std::ostream &os);
};

class StmtAST7: public BaseAST {
public:
    void generateIR(std::ostream &os);
};

class StmtAST8: public BaseAST {
public:
    StmtAST8(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> arrayAccess;
    std::shared_ptr<BaseAST> exp;
};

class BlockAST: public BaseAST {
public:
    BlockAST(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();

private:
    std::shared_ptr<BaseAST> blockItemList;
};

class BlockItemAST: public BaseAST {
public:
    BlockItemAST(BaseAST *itemt, bool isdecl);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
    //item is a DeclAST or ExtendStmtAST
    std::shared_ptr<BaseAST> item;
    bool isDecl;
};

class DeclAST: public BaseAST{
public:
    DeclAST(BaseAST *declt, bool isconst);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
private:
    // is a ConstDeclAST or a VarDeckAST
    std::shared_ptr<BaseAST> decl;
};

class ConstExpAST: public BaseAST{
public:
    ConstExpAST(BaseAST *expt);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
};

class ConstInitValAST: public BaseAST{
public:
    ConstInitValAST(BaseAST *constexp);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> constExp;
};

class ConstDefAST: public BaseAST{
public:
    ConstDefAST(std::string *id, BaseAST *val);
    void spreadSymbolTable();

private:
    std::shared_ptr<BaseAST> constInitVal;
};

class ConstDefAST2: public BaseAST{
public:
    ConstDefAST2(std::string *id, BaseAST *index, BaseAST *init);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    static void generateInitIR(std::ostream &os, std::string arrayBase, std::deque<std::shared_ptr<BaseAST>> index, std::deque<std::shared_ptr<BaseAST>> &init);
    static void generateAggregateIR(std::ostream &os, std::deque<std::shared_ptr<BaseAST>> index, std::deque<std::shared_ptr<BaseAST>> &init);
    std::shared_ptr<BaseAST> arrayIndexList;
    std::shared_ptr<BaseAST> constInitVal;
    std::deque<std::shared_ptr<BaseAST>> arrayIndexDeque;
    std::deque<std::shared_ptr<BaseAST>> constInitValDeque;
};

class ConstInitValAST2: public BaseAST{
public:
    ConstInitValAST2(BaseAST *c);
    void spreadSymbolTable();
    std::deque<std::shared_ptr<BaseAST>> constInitValDeque;
};

class ConstInitValListAST: public BaseAST{
public:
    ConstInitValListAST(BaseAST *v , BaseAST *l);
    std::deque<std::shared_ptr<BaseAST>> constInitValDeque;
};

class ConstDefListAST: public BaseAST{
public:
    ConstDefListAST(BaseAST *def);
    ConstDefListAST(BaseAST *list, BaseAST *def);
    std::deque<std::shared_ptr<BaseAST>> defList;
};

class ArrayIndexAST:public BaseAST{
public:
    ArrayIndexAST(BaseAST *e);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    std::shared_ptr<DimInfo> dim;
private:
    std::shared_ptr<BaseAST> exp;
};

class ArrayIndexListAST:public BaseAST{
public:
    ArrayIndexListAST(BaseAST *a, BaseAST *l);
    void spreadSymbolTable();

    //arrayindexast deque
    std::shared_ptr<BaseAST> arrayIndex;
    std::shared_ptr<BaseAST> arrayIndexList;
    std::shared_ptr<DimInfo> dim;
    // 全部的下标，每个都是ArrayIndexAST
    std::deque<std::shared_ptr<BaseAST>> arrayIndexDeque;
};

class ArrayAccessAST: public BaseAST{
public:
    ArrayAccessAST(std::string *id, BaseAST *a);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    std::string ident;
    std::shared_ptr<BaseAST> arrayIndexList;
    int dimNum;
};


class ConstDeclAST: public BaseAST{
public:
    ConstDeclAST(std::string *btype, BaseAST *list);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::string type;
    std::deque<std::shared_ptr<BaseAST>> constDefList;
};

class VarDefAST: public BaseAST{
public:
    VarDefAST(std::string *id);
    VarDefAST(std::string *id, BaseAST *init);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
private:
    std::shared_ptr<BaseAST> initVal;
    bool isInit;
};

class VarDefAST2: public BaseAST{
public:
    VarDefAST2(std::string *id, BaseAST *index, BaseAST *init);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    static void generateInitIR(std::ostream &os, std::string arrayBase, std::deque<std::shared_ptr<BaseAST>> index, std::deque<std::shared_ptr<BaseAST>> &init);
    static void generateAggregateIR(std::ostream &os, std::deque<std::shared_ptr<BaseAST>> index, std::deque<std::shared_ptr<BaseAST>> &init);
    std::shared_ptr<BaseAST> arrayIndexList;
    std::shared_ptr<BaseAST> initVal;
    std::deque<std::shared_ptr<BaseAST>> arrayIndexDeque;
    std::deque<std::shared_ptr<BaseAST>> initValDeque;
};

class InitValAST: public BaseAST{
public:
    InitValAST(BaseAST *expt);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
private:
    std::shared_ptr<BaseAST> exp;
};

class InitValAST2: public BaseAST{
public:
    InitValAST2(BaseAST *c);
    void spreadSymbolTable();
    std::deque<std::shared_ptr<BaseAST>> initValDeque;
};

class InitValListAST: public BaseAST{
public:
    InitValListAST(BaseAST *v , BaseAST *l);
    std::deque<std::shared_ptr<BaseAST>> initValDeque;
};


//only used to construct VarDeclAST, not used after that
class VarDefListAST: public BaseAST{
public:
    VarDefListAST(BaseAST *def);
    VarDefListAST(BaseAST *list, BaseAST *def);
    std::deque<std::shared_ptr<BaseAST>> defList;
};

class VarDeclAST: public BaseAST{
public:
    VarDeclAST(std::string *btype, BaseAST *list);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::string type;
    std::deque<std::shared_ptr<BaseAST>> varDefList;
};



class BlockItemListAST: public BaseAST {
public:
    BlockItemListAST();
    BlockItemListAST(BaseAST *blockItemsTemp, BaseAST *blockItemTemp);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
    std::deque<std::shared_ptr<BaseAST>> itemList;
};


class ExpAST: public ExpBaseAST {
public:
    ExpAST(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> lOrExp;
};

class ExpBaseAST1: public ExpBaseAST{
public:
    ExpBaseAST1(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> delegate;
};


class ExpBaseAST2: public ExpBaseAST{
public:
    ExpBaseAST2(BaseAST *ast1, BaseAST *ast2, const std::string &opt);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
protected:
    std::shared_ptr<BaseAST> left;
    std::shared_ptr<BaseAST> right;
    std::string op;
};

using LOrExpAST1 = ExpBaseAST1;

class LOrExpAST2: public ExpBaseAST{
public:
    LOrExpAST2(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> left;
    std::shared_ptr<BaseAST> right;
};

using LAndExpAST1 = ExpBaseAST1;

class LAndExpAST2: public ExpBaseAST{
public:
    LAndExpAST2(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> left;
    std::shared_ptr<BaseAST> right;
};

using EqExpAST1 = ExpBaseAST1;

using EqExpAST2 = ExpBaseAST2;

using RelExpAST1 = ExpBaseAST1;

using RelExpAST2 = ExpBaseAST2;

using AddExpAST1 = ExpBaseAST1;

using AddExpAST2 = ExpBaseAST2;

using MulExpAST1 = ExpBaseAST1;

using MulExpAST2 = ExpBaseAST2;

class UnaryExpAST1: public ExpBaseAST{
public:
    UnaryExpAST1(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> primaryExp;
};

class UnaryExpAST2: public ExpBaseAST{
public:
    UnaryExpAST2(std::string *opt, BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> unaryExp;
    std::string op;
};

//函数调用
class UnaryExpAST3: public ExpBaseAST{
public:
    UnaryExpAST3(std::string *id, BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::string ident;
    std::shared_ptr<BaseAST> funcRParamList;
    bool isVoid;
};

class FuncRParamListAST: public BaseAST{
public:
    FuncRParamListAST(BaseAST *ast1, BaseAST *ast2);
    void generateIR(std::ostream &os);
    //生成括号里内容
    void generateParam(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
    std::shared_ptr<BaseAST> funcRParamList;
};

class PrimaryExpAST1: public ExpBaseAST{
public:
    // although false and without explictly assign its name, no exp will invoke its name.
    PrimaryExpAST1(int n);
    void spreadSymbolTable();
};

class PrimaryExpAST2: public ExpBaseAST{
public:
    PrimaryExpAST2(BaseAST *ast);
    void generateIR(std::ostream &os);
    void spreadSymbolTable();
private:
    std::shared_ptr<BaseAST> exp;
};

class PrimaryExpAST3: public BaseAST{
public:
    // although false and without explictly assign its name, no exp will invoke its name.
    PrimaryExpAST3(std::string *id);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
private:
    std::string ident;
};

class PrimaryExpAST4: public BaseAST{
public:
    // although false and without explictly assign its name, no exp will invoke its name.
    PrimaryExpAST4(BaseAST *a);
    void spreadSymbolTable();
    void generateIR(std::ostream &os);
private:
    std::shared_ptr<BaseAST> arrayAccess;
};


#endif //COMPILER_AST_H
