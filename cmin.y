/****************************************************/
/* File: CMIN.y                                   */
/* The C-Minus Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
// #include "parse.h"

#define YYSTYPE TreeNode *
static int savedNumber;
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); /* evitar conflitos (editar dps) */

%}

%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token LT LE GT GE EQ NE SEMI
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%left PLUS MINUS
%left TIMES OVER COMMA
%right ASSIGN
%token ERROR

%% /* Grammar for TINY */

programa : declaracao_lista { savedTree = $1; }
        ;

declaracao_lista : declaracao_lista declaracao
                   { YYSTYPE t = $1;
                     if (t != NULL) {
                       while (t->sibling != NULL) { t = t->sibling; }
                       t->sibling = $2;
                       $$ = $1;
                     } else {
                       $$ = $2;
                     }
                   }
                 | declaracao { $$ = $1; }
                 ;

declaracao : var_declaracao { $$ = $1; }
            | fun_declaracao { $$ = $1; }
            ;

id : ID
     { savedName = copyString(tokenString);
       savedLineNo = lineno;
     }
   ;

num : NUM
      { savedNumber = atoi(tokenString);
        savedLineNo = lineno;
      }
    ;

var_declaracao : tipo_especificador id SEMI
                  { $$ = newDeclNode(VarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->attr.name = savedName;
                  }
                | tipo_especificador id LBRACKET num RBRACKET SEMI
                  { $$ = newDeclNode(ArrVarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->type = IntegerArray;
                    $$->attr.arr.name = savedName;
                    $$->attr.arr.size = savedNumber;
                  }
                ;

tipo_especificador : INT
                 { $$ = newExpNode(TypeK);
                   $$->type = Integer;
                 }
               | VOID
                 { $$ = newExpNode(TypeK);
                   $$->type = Void;
                 }
               ;

fun_declaracao : tipo_especificador id {
                   $$ = newDeclNode(FunK);
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                   // escopo
                 }
                 LPAREN params RPAREN composto_decl
                 { $$ = $3;
                   $$->child[0] = $1;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
                ;

params : param_lista { $$ = $1; }
       | VOID
         { $$ = newDeclNode(ParamK);
           $$->type = Void;
         }
       ;

param_lista : param_lista COMMA param
             { YYSTYPE t = $1;
               if (t != NULL) {
                 while (t->sibling != NULL) { t = t->sibling; }
                 t->sibling = $3;
                 $$ = $1;
               } else {
                 $$ = $2;
               }
             }
           | param { $$ = $1; }
           ;

param : tipo_especificador id
        { $$ = newDeclNode(ParamK);
          $$->child[0] = $1;
          $$->attr.name = savedName;
        }
      | tipo_especificador id LBRACKET RBRACKET
        { $$ = newDeclNode(ArrParamK);
          $$->child[0] = $1;
          $$->attr.name = copyString(savedName);
        }
      ;

composto_decl : LBRACE local_declaracoes param_lista RBRACE
                { $$ = newStmtNode(CompoundK);
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                }
              ;

local_declaracoes : local_declaracoes var_declaracao
                     { YYSTYPE t = $1;
                       if (t != NULL) {
                         while (t->sibling != NULL) { t = t->sibling; }
                         t->sibling = $2;
                         $$ = $1;
                       } else {
                         $$ = $2;
                       }
                     }
                   | { $$ = NULL; }
                   ;

param_lista : param_lista param
                 { YYSTYPE t = $1;
                   if (t != NULL) {
                     while (t->sibling != NULL) { t = t->sibling; }
                    t->sibling = $2;
                    $$ = $1;
                   } else {
                     $$ = $2;
                   }
                 }
               | { $$ = NULL; }
               ;

param : expressao_decl { $$ = $1; }
          | composto_decl { $$ = $1; }
          | selecao_decl { $$ = $1; }
          | iteracao_decl { $$ = $1; }
          | retorno_decl { $$ = $1; }
          ;

expressao_decl : expressao SEMI { $$ = $1; }
                | SEMI { $$ = NULL; }
                ;

selecao_decl : IF LPAREN expressao RPAREN param
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
               | IF LPAREN expressao RPAREN param ELSE param
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
               ;

iteracao_decl : WHILE LPAREN expressao RPAREN param
                 { $$ = newStmtNode(WhileK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
               ;

retorno_decl : RETURN SEMI
              { $$ = newStmtNode(ReturnK);
                $$->type = Void;
              }
            | RETURN expressao SEMI
              { $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
              }
            ;

expressao : var ASSIGN expressao
             { $$ = newStmtNode(AssignK);
               $$->child[0] = $1;
               $$->child[1] = $3;
             }
           | simples_expressao { $$ = $1; }
           ;

var : id
      { $$ = newExpNode(IdK);
        $$->attr.name = savedName;
      }
    | id {
        $$ = newExpNode(ArrIdK);
        $$->attr.name = savedName;
      } LBRACKET expressao RBRACKET
      {
        $$ = $2;
        $$->child[0] = $4;
      }
    ;

simples_expressao : soma_expressao relacional soma_expressao
                    { $$ = newExpNode(CalcK);
                      $$->child[0] = $1;
                      $$->child[1] = $2;
                      $$->child[2] = $3;
                    }
                  | soma_expressao { $$ = $1; }
                  ;

relacional : LT
        { $$ = newExpNode(OpK);
          $$->attr.op = LT;
        }
      | LE
        { $$ = newExpNode(OpK);
          $$->attr.op = LE;
        }
      | GT
        { $$ = newExpNode(OpK);
          $$->attr.op = GT;
        }
      | GE
        { $$ = newExpNode(OpK);
          $$->attr.op = GE;
        }
      | EQ
        { $$ = newExpNode(OpK);
          $$->attr.op = EQ;
        }
      | NE
        { $$ = newExpNode(OpK);
          $$->attr.op = NE;
        }
      ;

soma_expressao : soma_expressao soma termo
                      { $$ = newExpNode(CalcK);
                        $$->child[0] = $1;
                        $$->child[1] = $2;
                        $$->child[2] = $3;
                      }
                    | termo { $$ = $1; }
                    ;

soma : PLUS
        { $$ = newExpNode(OpK);
          $$->attr.op = PLUS;
        }
      | MINUS
        { $$ = newExpNode(OpK);
          $$->attr.op = MINUS;
        }
      ;

termo : termo mult fator
       { $$ = newExpNode(CalcK);
         $$->child[0] = $1;
         $$->child[1] = $2;
         $$->child[2] = $3;
       }
     | fator { $$ = $1; }
     ;

mult : TIMES
        { $$ = newExpNode(OpK);
          $$->attr.op = TIMES;
        }
      | OVER
        { $$ = newExpNode(OpK);
          $$->attr.op = OVER;
        }
      ;

fator : LPAREN expressao RPAREN { $$ = $2; }
       | var { $$ = $1; }
       | ativacao { $$ = $1; }
       | num
         { $$ = newExpNode(ConstK);
           $$->type = Integer;
           $$->attr.val = atoi(tokenString);
         };
       ;

ativacao : id {
         $$ = newExpNode(CallK);
         $$->attr.name = savedName;
         } LPAREN args RPAREN {
           $$ = $2;
           $$->child[0] = $4;
       }
     ;

args : arg_lista { $$ = $1; }
     | { $$ = NULL; }
     ;

arg_lista : arg_lista COMMA expressao
           { YYSTYPE t = $1;
             if (t != NULL) {
               while (t->sibling != NULL) { t = t->sibling; }
               t->sibling = $3;
               $$ = $1;
             } else {
               $$ = $3;
             }
           }
         | expressao { $$ = $1; }
         ;

%%

int yyerror(char * message) {
  if(Error) {
    return 0;
  }
  fprintf(listing,"ERRO SINTATICO: %s LINHA: %d\n", tokenString, lineno);
  Error = TRUE;
  return 0;
}

/* yylex ativacaos getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

