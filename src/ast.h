//
// Created by ruizhe on 24-4-13.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <fstream>
#include <memory>
#include <iostream>

// 所有 AST 的基类
class BaseAST {
public:
    virtual void generateIR(std::ostream &os) = 0;
    virtual ~BaseAST() = default;
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

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    FuncDefAST(BaseAST *ft, std::string *s, BaseAST *b):
        func_type(ft), funcName(*s), block(b){}

    void generateIR(std::ostream &os){
        os << "fun @" << funcName << "()";
        func_type->generateIR(os);
        os << " {\n";
        block->generateIR(os);
        os << "}";
    }

private:
    
    std::unique_ptr<BaseAST> func_type;
    std::string funcName;
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

class BlockAST: public BaseAST {
public:
    BlockAST(BaseAST *ast):stmt(ast){}

    void generateIR(std::ostream &os){
        os << "%"<< blockName <<":\n";
        stmt->generateIR(os);
    }

    void assignBlockName(std::string name){
        blockName = name;
    }

private:
    std::unique_ptr<BaseAST> stmt;
    // assign when reduce by sysy.y
    std::string blockName;
};

class StmtAST: public BaseAST {
public:
    StmtAST(int n):return_num(n){}

    void generateIR(std::ostream &os){
        os << "\t";
        os << "ret "<<return_num<<"\n";
    }

private:
    int return_num;
};

#endif //COMPILER_AST_H
