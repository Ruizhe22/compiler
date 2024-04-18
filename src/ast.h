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
    virtual void generateIR(std::ostream &os){};
    virtual ~BaseAST() = default;
    std::string name;
    bool isNum = false;
    int num;
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
    ExpBaseAST(bool fresh){
        if(fresh){
            name = "%"+std::to_string(++ExpBaseAST::expNum);
        }
    }
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

class BlockAST: public BaseAST {
public:
    BlockAST(BaseAST *ast):stmt(ast){}

    void generateIR(std::ostream &os){
        os << name <<":\n";
        stmt->generateIR(os);
    }

    void assignBlockName(std::string name_t){
        name = name_t;
    }

private:
    std::unique_ptr<BaseAST> stmt;
    // assign when reduce by sysy.y
    std::string name;
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

private:
    std::unique_ptr<BaseAST> exp;
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

private:
    std::unique_ptr<BaseAST> lOrExp;

};

class ExpBaseAST1: public ExpBaseAST{
public:
    ExpBaseAST1(BaseAST *ast):ExpBaseAST(false),delegate(ast){
        if(delegate->isNum){
            isNum = true;
            num = delegate->num;
        }
        name = delegate->name;
    }

    void generateIR(std::ostream &os){
        delegate->generateIR(os);
    }

private:
    std::unique_ptr<BaseAST> delegate;
};


class ExpBaseAST2: public ExpBaseAST{
public:
    ExpBaseAST2(BaseAST *ast1, BaseAST *ast2, const std::string &opt):ExpBaseAST(true),left(ast1),right(ast2),op(opt){}

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

protected:
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;
    std::string op;
};

using LOrExpAST1 = ExpBaseAST1;

class LOrExpAST2: public ExpBaseAST{
public:
    LOrExpAST2(BaseAST *ast1, BaseAST *ast2):ExpBaseAST(true),left(ast1),right(ast2){}

    void generateIR(std::ostream &os){
        std::string temp1 = "%_l" + std::to_string(ExpBaseAST::expNum);
        std::string temp2 = "%_r" + std::to_string(ExpBaseAST::expNum);
        if(left->isNum && right->isNum){
            os << "\t" << temp1 << " = eq 0," << left->num << "\n";
            os << "\t" << temp2 << " = eq 0," << right->num << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else if(!left->isNum && right->isNum){
            left->generateIR(os);
            os << "\t" << temp1 << " = eq 0," << left->name << "\n";
            os << "\t" << temp2 << " = eq 0," << right->num << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else if(left->isNum && !right->isNum){
            right->generateIR(os);
            os << "\t" << temp1 << " = eq 0," << left->num << "\n";
            os << "\t" << temp2 << " = eq 0," << right->name << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
        else{
            left->generateIR(os);
            right->generateIR(os);
            os << "\t" << temp1 << " = eq 0," << left->name << "\n";
            os << "\t" << temp2 << " = eq 0," << right->name << "\n";
            os << "\t" << name << " =  or " << temp1 <<", " << temp2 << "\n";
        }
    }

private:
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;

};

using LAndExpAST1 = ExpBaseAST1;

class LAndExpAST2: public ExpBaseAST{
public:
    LAndExpAST2(BaseAST *ast1, BaseAST *ast2):ExpBaseAST(true),left(ast1),right(ast2){}

    void generateIR(std::ostream &os){
        std::string temp1 = "%_l" + std::to_string(ExpBaseAST::expNum);
        std::string temp2 = "%_r" + std::to_string(ExpBaseAST::expNum);
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
        if(primaryExp->isNum){
            isNum = true;
            num = primaryExp->num;
        }
        name = primaryExp->name;
    }

    void generateIR(std::ostream &os){
        primaryExp->generateIR(os);
    }

private:
    std::unique_ptr<BaseAST> primaryExp;
};

class UnaryExpAST2: public ExpBaseAST{
public:
    UnaryExpAST2(std::string *opp, BaseAST *ast):ExpBaseAST(true),unaryExp(ast),op(*opp){
        if(unaryExp->isNum){
            isNum = true;
            if(op == "add"){
                num = unaryExp->num;
            }
            else if(op == "sub"){
                num = -unaryExp->num;
            }
            else if(op == "eq"){
                num = (0==unaryExp->num);
            }
        }
        else if(op=="add"){
            --ExpBaseAST::expNum;
            name = unaryExp->name;
        }
    }

    void generateIR(std::ostream &os){
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

private:
    std::unique_ptr<BaseAST> exp;
};

#endif //COMPILER_AST_H
