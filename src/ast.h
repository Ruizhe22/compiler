//
// Created by ruizhe on 24-4-13.
//

#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <fstream>
#include <memory>

// 所有 AST 的基类
class BaseAST {
public:
    virtual void generalIR(std::ostream &os) = 0;
    virtual ~BaseAST() = default;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    CompUnitAST(BaseAST *f):func_def(f){}
    std::unique_ptr<BaseAST> func_def;

    void generalIR(std::ostream &os){
        func_def->generalIR(os);
    }

};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    FuncDefAST(BaseAST *ft, std::string *s, BaseAST *b):
        func_type(ft), ident(*s), block(b){}

    void generalIR(std::ostream &os){
        os << "fun @" << ident << "()";
        func_type->generalIR(os);
        os << " {\n";
        block->generalIR(os);
        os << "}";
    }

    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
};

class FuncTypeAST: public BaseAST {
public:
    FuncTypeAST(const std::string &t):type(t){}

    void generalIR(std::ostream &os){
        os << ":" << type;
    }

    std::string type;
};

class BlockAST: public BaseAST {
public:
    BlockAST(BaseAST *ast):stmt(ast){}

    void generalIR(std::ostream &os){
        os << "%entry:\n";
        stmt->generalIR(os);
    }

    std::unique_ptr<BaseAST> stmt;
};

class StmtAST: public BaseAST {
public:
    StmtAST(int n):return_num(n){}

    void generalIR(std::ostream &os){
        os << "\t";
        os << "ret "<<return_num<<"\n";
    }

    int return_num;
};

#endif //COMPILER_AST_H
