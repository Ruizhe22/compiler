%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST PLUS MINUS NOT OR AND EQ NE LT GT LE GE MUL DIV MOD
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block BlockItemList BlockItem Stmt Exp LOrExp LAndExp EqExp RelExp AddExp MulExp UnaryExp PrimaryExp Decl ConstExp ConstInitVal ConstDef ConstDefList ConstDecl
%type <int_val> Number
%type <str_val> UnaryOp BType LVal

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    $$ = new FuncDefAST($1,$2,$5);
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    $$ = new FuncTypeAST("i32");
  }
  ;

Block
    : '{' BlockItemList '}' {
        auto block = new BlockAST($2);
        block->assignBlockName("%entry");
        $$ = block;
    }
    ;

BlockItemList
    : {
        $$ = new BlockItemListAST();
    }
    | BlockItemList BlockItem {
        $$ = new BlockItemListAST($1,$2);
    }
    ;

BlockItem
    : Decl {
        $$ = new BlockItemAST($1,true);
    }
    | Stmt {
        $$ = new BlockItemAST($1,false);
    }
    ;

Stmt
  : RETURN Exp ';' {
    $$ = new StmtAST($2);
  }
  ;



Number
  : INT_CONST {
    $$ = $1;
  }
  ;

Exp
    : LOrExp {
        $$ = new ExpAST($1);
    }
    ;

LOrExp
    : LAndExp {
        $$ = new LOrExpAST1($1);
    }
    | LOrExp OR LAndExp {
        $$ = new LOrExpAST2($1, $3);
    }
    ;

LAndExp
    : EqExp {
        $$ = new LAndExpAST1($1);
    }
    | LAndExp AND EqExp {
        $$ = new LAndExpAST2($1, $3);
    };

EqExp
    : RelExp {
        $$ = new EqExpAST1($1);
    }
    | EqExp EQ RelExp {
        $$ = new EqExpAST2($1, $3, "eq");
    }
    | EqExp NE RelExp {
        $$ = new EqExpAST2($1, $3, "ne");
    }
    ;

RelExp
     : AddExp {
        $$ = new RelExpAST1($1);
     }
     | RelExp LT AddExp{
        $$ = new RelExpAST2($1, $3, "lt");
     }
     | RelExp GT AddExp{
        $$ = new RelExpAST2($1, $3, "gt");
     }
     | RelExp LE AddExp{
        $$ = new RelExpAST2($1, $3, "le");
     }
     | RelExp GE AddExp{
        $$ = new RelExpAST2($1, $3, "ge");
     }
     ;


AddExp
    : MulExp {
        $$ = new AddExpAST1($1);
    }
    | AddExp PLUS MulExp
    {
        $$ = new AddExpAST2($1, $3, "add");
    }
    | AddExp MINUS MulExp {
        $$ = new AddExpAST2($1, $3, "sub");
    }
    ;

MulExp
    : UnaryExp {
        $$ = new MulExpAST1($1);
    }
    | MulExp MUL UnaryExp {
        $$ = new MulExpAST2($1, $3, "mul");
    }
    | MulExp DIV UnaryExp {
        $$ = new MulExpAST2($1, $3, "div");
    }
    | MulExp MOD UnaryExp {
        $$ = new MulExpAST2($1, $3, "mod");
    }
    ;

UnaryOp
    : PLUS {
        $$ = new std::string("add");
    }
    | MINUS {
        $$ = new std::string("sub");
    }
    | NOT {
        $$ = new std::string("eq");
    }
    ;


UnaryExp
    : PrimaryExp  {
        $$ = new UnaryExpAST1($1);
    }
    | UnaryOp UnaryExp {
        $$ = new UnaryExpAST2($1, $2);
    }
    ;

PrimaryExp
    : Number {
        $$ = new PrimaryExpAST1($1);
    }
    | '(' Exp ')' {
        $$ = new PrimaryExpAST2($2);
    }
    | LVal {
        $$ = new PrimaryExpAST3($1);
    }
    ;


Decl
    : ConstDecl {
        $$ = new DeclAST($1,true);
    }
    ;


ConstDecl
    : CONST BType ConstDef ConstDefList ';' {
        $$ = new ConstDeclAST($2, $3, $4);
    }
    ;

ConstDefList
    : {
        $$ = new ConstDefListAST();
    }
    | ConstDefList ',' ConstDef {
        $$ = new ConstDefListAST($1,$3);
    }
    ;

ConstDef
    : IDENT '=' ConstInitVal{

        $$ = new ConstDefAST($1,$3);
    }
    ;

BType
    : INT {
        $$ = new std::string("int");
    }
    ;

ConstInitVal
    : ConstExp{
        $$ = new ConstInitValAST($1);
    }
    ;

LVal
    : IDENT {
       $$ = $1;
    }
    ;

ConstExp
    : Exp{
        $$ = new ConstExpAST($1);
    }
    ;








%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
