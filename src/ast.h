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

class SymbolInfo{
public:
    SymbolInfo(std::string namet, std::string typet, bool isConstt, int constNumt)
        :name(namet),type(typet),isNum(isConstt),constNum(constNumt){}

    std::string name;
    bool isNum;
    std::string type;
    int constNum;
};

// 所有 AST 的基类
class BaseAST {
public:
    virtual void generateIR(std::ostream &os){ return; }
    virtual void spreadSymbolTable(){ return; }
    virtual ~BaseAST() = default;
    std::string name;
    bool isNum = false;
    bool isID = false;
    int num;
    std::unordered_map<std::string, std::shared_ptr<SymbolInfo>> symbolTable;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    CompUnitAST(BaseAST *f):func_def(f){}


    void generateIR(std::ostream &os){
        func_def->generateIR(os);
    }

private:
    std::unique_ptr<BaseAST> func_def;

};

class ExpBaseAST: public BaseAST {
public:
    //restore to 0 in funcDef construct
    static int expNum;
    // if not change expNum, child need to assign name in the body
    ExpBaseAST(bool fresh):allocNewName(fresh){
    }

    bool allocNewName;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    FuncDefAST(BaseAST *ft, std::string *s, BaseAST *b):
            func_type(ft), name("@" + *s), block(b){
        // exp restart to count from 0 in a new func
        ExpBaseAST::expNum = 0;
    }

    void generateIR(std::ostream &os){
        os << "fun " << name << "()";
        func_type->generateIR(os);
        os << " {\n";
        block->generateIR(os);
        os << "}";
    }

private:

    std::unique_ptr<BaseAST> func_type;
    std::string name;
    std::unique_ptr<BaseAST> block;
};

class FuncTypeAST: public BaseAST {
public:
    FuncTypeAST(const std::string &t):type(t){}

    void generateIR(std::ostream &os){
        os << ":" << type;
    }
private:
    std::string type;
};

class StmtAST: public BaseAST {
public:
    StmtAST(BaseAST *ast):exp(ast){}

    void generateIR(std::ostream &os){
        if(exp->isNum){
            os << "\tret "<< exp->num <<"\n";
        }
        else{
            exp->generateIR(os);
            os << "\tret "<< exp->name <<"\n";
        }
    }

    void spreadSymbolTable(){
        exp->symbolTable = symbolTable;
        exp->spreadSymbolTable();
        if(exp->isNum){
            isNum = true;
            num = exp->num;
        }
    }

private:
    std::unique_ptr<BaseAST> exp;
};

class BlockAST: public BaseAST {
public:
    BlockAST(BaseAST *ast):BlockItemListAST(ast){
        //update child block symbol table
    }

    void generateIR(std::ostream &os){
        os << name <<":\n";
        BlockItemListAST->generateIR(os);
    }

    void assignBlockName(std::string name_t){
        name = name_t;
    }

private:
    std::unique_ptr<BaseAST> BlockItemListAST;
    // assign when reduce by sysy.y
    std::string name;

};

class BlockItemAST: public BaseAST {
public:
    BlockItemAST(BaseAST *itemt, bool isdecl):item(itemt),isDecl(isdecl){
        symbolTable = item-> symbolTable;
    }

    void generateIR(std::ostream &os){
        item->generateIR(os);
    }

    std::shared_ptr<BaseAST> item;
    bool isDecl;
};

class DeclAST: public BaseAST{
public:
    DeclAST(BaseAST *declt, bool isconst):decl(declt),isNum(isconst){
        symbolTable = decl->symbolTable;
    }

    void spreadSymbolTable(){
        // 先传下去 decl语句之前的符号表
        decl->symbolTable = symbolTable;
        decl->spreadSymbolTable();
        // 再传回 decl语句起作用之后的符号表
        symbolTable = decl->symbolTable;
    }

private:
    std::shared_ptr<BaseAST> decl;
    bool isNum;
};



class ConstExpAST: public BaseAST{
public:
    ConstExpAST(BaseAST *expt):exp(expt){
        // without error detection
        isNum = exp->isNum;
        num = exp->num;
        name = exp->name;
    }
    void spreadSymbolTable(){
        exp->symbolTable = symbolTable;
        exp->spreadSymbolTable();
        if(exp->isNum){
            num = exp->num;
            isNum = true;
        }
    }

private:
    std::unique_ptr<BaseAST> exp;
};

class ConstInitValAST: public BaseAST{
public:
    ConstInitValAST(BaseAST *constexp):constExp(constexp){
        isNum = constExp->isNum;
        num = constExp->num;
        name = constExp->name;
    }
    void spreadSymbolTable(){
        constExp->symbolTable = symbolTable;
        constExp->spreadSymbolTable();
        if(constExp->isNum){
            num = constExp->num;
            isNum = true;
        }
    }
private:
    std::unique_ptr<BaseAST> constExp;
};

class ConstDefAST: public BaseAST{
public:
    ConstDefAST(std::string *id, BaseAST *val):constInitVal(val){
        name = "@" + *id;
        num = constInitVal->num;
        isNum = constInitVal->isNum;
    }
    void spreadSymbolTable(){
        constInitVal->symbolTable = symbolTable;
        constInitVal->spreadSymbolTable();
        if(constInitVal->isNum){
            num = constInitVal->num;
            isNum = true;
            symbolTable[name]->constNum = num;
        }
    }
private:
    std::unique_ptr<BaseAST> constInitVal;
};

class ConstDefListAST: public BaseAST{
public:
    ConstDefListAST() {
    };
    ConstDefListAST(BaseAST *list, BaseAST *def):defList(((ConstDefListAST *)list)->defList){
        defList.push_back(std::shared_ptr<ConstDefAST>((ConstDefAST *)def));
    }

    std::deque<std::shared_ptr<BaseAST>> defList;
};

class ConstDeclAST: public BaseAST{
public:
    ConstDeclAST(std::string *btype, BaseAST *def, BaseAST *list):type(*btype){
        constDefList = ((ConstDefListAST *)list)->defList;
        constDefList.push_front(std::shared_ptr<BaseAST>(def));
        def->symbolTable[def->name] = std::make_shared<SymbolInfo>(def->name,type,true,def->num);
        for(const auto &d : ((ConstDefListAST *)list)->defList){
            d->symbolTable[d->name] = std::make_shared<SymbolInfo>(d->name,type,true,d->num);
        }

    }

    void spreadSymbolTable(){
        // 先传下去 decl语句之前的符号表 对于每个新的符号
        for(const auto &cd : constDefList){
            cd->symbolTable.merge(symbolTable);
            //std::cout<<"#### "<< cd->symbolTable["@a"]->constNum  << std::endl;
            cd->spreadSymbolTable();
            symbolTable = cd->symbolTable;
        }
    }

private:
    std::string type;
    std::deque<std::shared_ptr<BaseAST>> constDefList;
};


class BlockItemListAST: public BaseAST {
public:
    BlockItemListAST() = default;
    BlockItemListAST(BaseAST *blockItemsTemp, BaseAST *blockItemTemp):itemList(((BlockItemListAST *)blockItemsTemp)->itemList){
        auto temp = (BlockItemAST *)blockItemTemp;
        itemList.push_back(temp->item);
        // update symbol table of item and item list
        if(temp->isDecl){
            std::shared_ptr<DeclAST> decl = std::dynamic_pointer_cast<DeclAST>(temp->item);
            decl->symbolTable = symbolTable;
            decl->spreadSymbolTable();
            symbolTable.merge(decl->symbolTable);
            symbolTable.merge(((BlockItemListAST *)blockItemsTemp)->symbolTable);
        }
        else{
            symbolTable.merge(temp->symbolTable);
            symbolTable.merge(((BlockItemListAST *)blockItemsTemp)->symbolTable);
            std::shared_ptr<StmtAST> stmt = std::dynamic_pointer_cast<StmtAST>(temp->item);
            stmt->symbolTable = symbolTable;
            stmt->spreadSymbolTable();
        }

    }

    void generateIR(std::ostream &os){
        for(const auto &item : itemList){
            item->generateIR(os);
        }
    }

    std::deque<std::shared_ptr<BaseAST>> itemList;
};


class ExpAST: public ExpBaseAST {
public:

    ExpAST(BaseAST *ast):ExpBaseAST(false),lOrExp(ast){
        if(lOrExp->isNum){
            isNum = true;
            num = lOrExp->num;
        }
        name = lOrExp->name;
    }

    void generateIR(std::ostream &os){
        lOrExp->generateIR(os);
    }

    void spreadSymbolTable(){
        lOrExp->symbolTable = symbolTable;
        lOrExp->spreadSymbolTable();
        if(lOrExp->isNum){
            isNum = true;
            num = lOrExp->num;
        }
    }

private:
    std::unique_ptr<BaseAST> lOrExp;

};

class ExpBaseAST1: public ExpBaseAST{
public:
    ExpBaseAST1(BaseAST *ast):ExpBaseAST(false),delegate(ast){
        name = delegate->name;
        if(delegate->isNum){
            isNum = true;
            num = delegate->num;
        }
    }

    void generateIR(std::ostream &os){
        delegate->generateIR(os);
    }

    void spreadSymbolTable(){
        delegate->symbolTable = symbolTable;
        delegate->spreadSymbolTable();
        if(delegate->isNum){
            isNum = true;
            num = delegate->num;
        }
    }

private:
    std::unique_ptr<BaseAST> delegate;
};


class ExpBaseAST2: public ExpBaseAST{
public:
    ExpBaseAST2(BaseAST *ast1, BaseAST *ast2, const std::string &opt):ExpBaseAST(true),left(ast1),right(ast2),op(opt){
        if(left->isNum && right->isNum){
            isNum = true;
            if(op =="lt") { num = left->num < right->num; }
            if(op =="gt") { num = left->num > right->num; }
            if(op =="le") { num = left->num <= right->num; }
            if(op =="ge") { num = left->num >= right->num; }
            if(op =="add") { num = left->num + right->num; }
            if(op =="sub") { num = left->num - right->num; }
            if(op =="mul") { num = left->num * right->num; }
            if(op =="div") { num = left->num / right->num; }
            if(op =="mod") { num = left->num % right->num; }
            if(op =="eq") { num = left->num == right->num; }
            if(op =="ne") { num = left->num != right->num; }
        }
    }

    void generateIR(std::ostream &os){
        if(left->isNum && right->isNum){
            os << "\t" << name << " = " << op << " " << left->num <<", " << right->num << "\n";
        }
        else if(!left->isNum && right->isNum){
            left->generateIR(os);
            os << "\t" << name << " = " << op << " " << left->name <<", " << right->num << "\n";
        }
        else if(left->isNum && !right->isNum){
            right->generateIR(os);
            os << "\t" << name << " = " << op << " " << left->num <<", " << right->name << "\n";
        }
        else{
            left->generateIR(os);
            right->generateIR(os);
            os << "\t" << name << " = " << op << " " << left->name <<", " << right->name << "\n";
        }
    }

    void spreadSymbolTable(){
        left->symbolTable = symbolTable;
        left->spreadSymbolTable();
        right->symbolTable = symbolTable;
        right->spreadSymbolTable();
        if(left->isNum && right->isNum){
            isNum = true;
            if(op =="lt") { num = left->num < right->num; }
            if(op =="gt") { num = left->num > right->num; }
            if(op =="le") { num = left->num <= right->num; }
            if(op =="ge") { num = left->num >= right->num; }
            if(op =="add") { num = left->num + right->num; }
            if(op =="sub") { num = left->num - right->num; }
            if(op =="mul") { num = left->num * right->num; }
            if(op =="div") { num = left->num / right->num; }
            if(op =="mod") { num = left->num % right->num; }
            if(op =="eq") { num = left->num == right->num; }
            if(op =="ne") { num = left->num != right->num; }
        }
        else if(allocNewName){
            name = "%"+std::to_string(++ExpBaseAST::expNum);
        }
    }

protected:
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;
    std::string op;
};

using LOrExpAST1 = ExpBaseAST1;

class LOrExpAST2: public ExpBaseAST{
public:
    LOrExpAST2(BaseAST *ast1, BaseAST *ast2):ExpBaseAST(true),left(ast1),right(ast2){
        if(left->isNum && right->isNum){
            isNum = true;
            num = left->num || right->num;
        }
    }

    void generateIR(std::ostream &os){
        std::string temp1 = "%lorl" + name.substr(1);
        std::string temp2 = "%lorr" + name.substr(1);
        if(left->isNum && right->isNum){
            os << "\t" << temp1 << " = ne 0," << left->num << "\n";
            os << "\t" << temp2 << " = ne 0," << right->num << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else if(!left->isNum && right->isNum){
            left->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->name << "\n";
            os << "\t" << temp2 << " = ne 0," << right->num << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else if(left->isNum && !right->isNum){
            right->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->num << "\n";
            os << "\t" << temp2 << " = ne 0," << right->name << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else{
            left->generateIR(os);
            right->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->name << "\n";
            os << "\t" << temp2 << " = ne 0," << right->name << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
    }

    void spreadSymbolTable(){
        left->symbolTable = symbolTable;
        left->spreadSymbolTable();
        right->symbolTable = symbolTable;
        right->spreadSymbolTable();
        if(left->isNum && right->isNum){
            isNum = true;
            num = left->num || right->num;
        }
        else if(allocNewName){
            name = "%"+std::to_string(++ExpBaseAST::expNum);
        }
    }

private:
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;

};

using LAndExpAST1 = ExpBaseAST1;

class LAndExpAST2: public ExpBaseAST{
public:
    LAndExpAST2(BaseAST *ast1, BaseAST *ast2):ExpBaseAST(true),left(ast1),right(ast2){
        if(left->isNum && right->isNum){
            --ExpBaseAST::expNum;
            isNum = true;
            num = left->num && right->num;
        }
    }

    void generateIR(std::ostream &os){
        std::string temp1 = "%landl" + name.substr(1);
        std::string temp2 = "%landr" + name.substr(1) ;
        if(left->isNum && right->isNum){
            os << "\t" << temp1 << " = ne 0," << left->num << "\n";
            os << "\t" << temp2 << " = ne 0," << right->num << "\n";
            os << "\t" << name << " =  and " << temp1 <<", " << temp2 << "\n";
        }
        else if(!left->isNum && right->isNum){
            left->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->name << "\n";
            os << "\t" << temp2 << " = ne 0," << right->num << "\n";
            os << "\t" << name << " =  and " << temp1 <<", " << temp2 << "\n";
        }
        else if(left->isNum && !right->isNum){
            right->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->num << "\n";
            os << "\t" << temp2 << " = ne 0," << right->name << "\n";
            os << "\t" << name << " =  and " << temp1 <<", " << temp2 << "\n";
        }
        else{
            left->generateIR(os);
            right->generateIR(os);
            os << "\t" << temp1 << " = ne 0," << left->name << "\n";
            os << "\t" << temp2 << " = ne 0," << right->name << "\n";
            os << "\t" << name << " =  and " << temp1 <<", " << temp2 << "\n";
        }
    }

    void spreadSymbolTable(){
        left->symbolTable = symbolTable;
        left->spreadSymbolTable();
        right->symbolTable = symbolTable;
        right->spreadSymbolTable();
        if(left->isNum && right->isNum){
            isNum = true;
            num = left->num && right->num;
        }
        else if(allocNewName){
            name = "%"+std::to_string(++ExpBaseAST::expNum);
        }
    }

private:
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;
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
    UnaryExpAST1(BaseAST *ast):ExpBaseAST(false),primaryExp(ast){
        name = primaryExp->name;
        if(primaryExp->isNum){
            isNum = true;
            num = primaryExp->num;
        }
    }

    void generateIR(std::ostream &os){
        primaryExp->generateIR(os);
    }

    void spreadSymbolTable(){
        primaryExp->symbolTable = symbolTable;
        primaryExp->spreadSymbolTable();
        if(primaryExp->isNum){
            isNum = true;
            num = primaryExp->num;
        }
        /*
         else if(primaryExp->isID){
            // in symbol table not const id

        }
         */
    }

private:
    std::unique_ptr<BaseAST> primaryExp;
};

class UnaryExpAST2: public ExpBaseAST{
public:
    UnaryExpAST2(std::string *opt, BaseAST *ast):ExpBaseAST(true),unaryExp(ast),op(*opt){
        /*if(unaryExp->isNum){
            isNum = true;
            if(op == "add"){
                num = unaryExp->num;
            }
            else if(op == "sub"){
                num = -unaryExp->num;
            }
            else if(op == "not"){
                num = (0==unaryExp->num);
            }
        }
        else if(op=="add"){
            name = unaryExp->name;
        }*/
    }

    void generateIR(std::ostream &os){
        if(op=="not") op = "eq";
        if(unaryExp->isNum){
            if(op != "add") {
                os << "\t" << name << "= " << op << " 0, " << unaryExp->num << "\n";
            }
        }
        else{
            unaryExp->generateIR(os);
            if(op != "add") {
                os << "\t" << name << "= " << op << " 0, " << unaryExp->name << "\n";
            }
            else if(allocNewName){
                name = "%"+std::to_string(++ExpBaseAST::expNum);
            }
        }

    }

    void spreadSymbolTable(){
        unaryExp->symbolTable = symbolTable;
        unaryExp->spreadSymbolTable();
        if(unaryExp->isNum){
            isNum = true;
            if(op == "add"){
                num = unaryExp->num;
            }
            else if(op == "sub"){
                num = -unaryExp->num;
            }
            else if(op == "not"){
                num = (0==unaryExp->num);
            }
        }
    }

private:
    std::unique_ptr<BaseAST> unaryExp;
    std::string op;
};

class PrimaryExpAST1: public ExpBaseAST{
public:
    // although false and without explictly assign its name, no exp will invoke its name.
    PrimaryExpAST1(int n):ExpBaseAST(false){
        isNum = true;
        num = n ;
    }

    void generateIR(std::ostream &os){
        os << "\t" << name << "= add 0, " << num << "\n";
    }

    void spreadSymbolTable(){
        return;
    }
};

class PrimaryExpAST2: public ExpBaseAST{
public:
    PrimaryExpAST2(BaseAST *ast):ExpBaseAST(false),exp(ast){
        if(exp->isNum){
            isNum = true;
            num = exp->num;
        }
        name = exp->name;
    }

    void generateIR(std::ostream &os){
        exp->generateIR(os);
    }

    void spreadSymbolTable(){
        exp->symbolTable = symbolTable;
        exp->spreadSymbolTable();
        if(exp->isNum){
            isNum = true;
            num = exp->num;
        }
    }

private:
    std::unique_ptr<BaseAST> exp;
};

class PrimaryExpAST3: public BaseAST{
public:
    // although false and without explictly assign its name, no exp will invoke its name.
    PrimaryExpAST3(std::string *id){
        name = "@"+ *id;
        isID = true;
    }

    void spreadSymbolTable(){
        std::shared_ptr<SymbolInfo> symbol = symbolTable[name];
        if(symbol->isNum){
            isNum = true;
            num = symbol->constNum;
        }
        return;
    }
};




#endif //COMPILER_AST_H
