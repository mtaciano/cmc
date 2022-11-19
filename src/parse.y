/* Especificação C- Yacc/Bison */
/* TODO: consertar os conflitos de shift/reduce */

%{
#define YYPARSER /* Diferencia o output do Yacc de outros códigos de output */

#include "../src/common/globals.h"
#include "../src/common/util.h"
#include "../src/scan.h"
#include "../src/parse.h"

#define YYSTYPE TreeNode*

static int saved_number;
static char *saved_name;
static int saved_lineno;
static TreeNode *saved_tree; /* Árvore sintática para retornos */

static int yylex(void); /* Evitar conflitos */
int yyerror(char *message); /* Evitar conflitos */
%}

/* TODO: resolver os problemas de shift/reduce */

%token IF ELSE INT RETURN VOID WHILE
%token ID NUM
%token LT LE GT GE EQ NE SEMI
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%left PLUS MINUS
%left TIMES OVER COMMA
%right ASSIGN
%token ERROR

%% /* Gramática do C- */

programa : declaracao_lista { saved_tree = $1; }
        ;

declaracao_lista : declaracao_lista declaracao
                   {
                      YYSTYPE t = $1;

                    if (t != NULL) {
                        while (t->sibling != NULL) {
                            t = t->sibling;
                        }

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
    {
        saved_name = copy_string(token_string);
        saved_lineno = lineno;
    }
   ;

num : NUM
    {
        saved_number = atoi(token_string);
        saved_lineno = lineno;
    }
    ;

var_declaracao : tipo_especificador id SEMI
                {
                    $$ = new_DeclNode(VarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->attr.name = saved_name;
                }
                | tipo_especificador id LBRACKET num RBRACKET SEMI
                {
                    $$ = new_DeclNode(ArrVarK);
                    $$->child[0] = $1;
                    $$->lineno = lineno;
                    $$->type = IntegerArray;
                    $$->attr.arr.name = saved_name;
                    $$->attr.arr.size = saved_number;
                }
                ;

tipo_especificador : INT
                {
                    $$ = new_ExpNode(TypeK);
                    $$->type = Integer;
                }
               | VOID
                {
                    $$ = new_ExpNode(TypeK);
                    $$->type = Void;
                }
               ;

fun_declaracao : tipo_especificador id
                {
                    $$ = new_DeclNode(FunK);
                    $$->lineno = lineno;
                    $$->attr.name = saved_name;
                }
                 LPAREN params RPAREN composto_decl
                {
                    $$ = $3;
                    $$->child[0] = $1;
                    $$->child[1] = $5;
                    $$->child[2] = $7;
                }
                ;

params : param_lista { $$ = $1; }
       | VOID
        {
            $$ = new_DeclNode(ParamK);
            $$->attr.name = "void";
            $$->type = Void;
        }
       ;

param_lista : param_lista COMMA param
             {
                YYSTYPE t = $1;

                if (t != NULL) {
                    while (t->sibling != NULL) {
                        t = t->sibling;
                    }

                    t->sibling = $3;
                    $$ = $1;
                } else {
                    $$ = $2;
                }
             }
           | param { $$ = $1; }
           ;

param : tipo_especificador id
        {
            $$ = new_DeclNode(ParamK);
            $$->child[0] = $1;
            $$->attr.name = saved_name;
        }
      | tipo_especificador id LBRACKET RBRACKET
        {
            $$ = new_DeclNode(ArrParamK);
            $$->child[0] = $1;
            $$->attr.name = copy_string(saved_name);
        }
      ;

composto_decl : LBRACE local_declaracoes param_lista RBRACE
                {
                    $$ = new_StmtNode(CompoundK);
                    $$->child[0] = $2;
                    $$->child[1] = $3;
                }
              ;

local_declaracoes : local_declaracoes var_declaracao
                    {
                        YYSTYPE t = $1;

                        if (t != NULL) {
                            while (t->sibling != NULL) {
                                t = t->sibling;
                            }

                            t->sibling = $2;
                            $$ = $1;
                        } else {
                            $$ = $2;
                        }
                    }
                   | %empty { $$ = NULL; }
                   ;

param_lista : param_lista param
                {
                    YYSTYPE t = $1;

                    if (t != NULL) {
                        while (t->sibling != NULL) {
                            t = t->sibling;
                        }

                        t->sibling = $2;
                        $$ = $1;
                    } else {
                        $$ = $2;
                    }
                }
               | %empty { $$ = NULL; }
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
                {
                    $$ = new_StmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                }
               | IF LPAREN expressao RPAREN param ELSE param
                {
                    $$ = new_StmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                    $$->child[2] = $7;
                }
               ;

iteracao_decl : WHILE LPAREN expressao RPAREN param
                {
                    $$ = new_StmtNode(WhileK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                }
               ;

retorno_decl : RETURN SEMI
            {
                $$ = new_StmtNode(ReturnK);
                $$->type = Void;
            }
            | RETURN expressao SEMI
            {
                $$ = new_StmtNode(ReturnK);
                $$->child[0] = $2;
            }
            ;

expressao : var ASSIGN expressao
            {
                $$ = new_StmtNode(AssignK);
                $$->child[0] = $1;
                $$->child[1] = $3;
            }
           | simples_expressao { $$ = $1; }
           ;

var : id
    {
        $$ = new_ExpNode(IdK);
        $$->attr.name = saved_name;
    }
    | id
    {
        $$ = new_ExpNode(ArrIdK);
        $$->attr.name = saved_name;
    }
     LBRACKET expressao RBRACKET
    {
        $$ = $2;
        $$->child[0] = $4;
    }
    ;

simples_expressao : soma_expressao relacional soma_expressao
                    {
                        $$ = new_ExpNode(CalcK);
                        $$->child[0] = $1;
                        $$->child[1] = $2;
                        $$->child[2] = $3;
                    }
                  | soma_expressao { $$ = $1; }
                  ;

relacional : LT
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = LT;
        }
      | LE
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = LE;
        }
      | GT
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = GT;
        }
      | GE
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = GE;
        }
      | EQ
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = EQ;
        }
      | NE
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = NE;
        }
      ;

soma_expressao : soma_expressao soma termo
                {
                    $$ = new_ExpNode(CalcK);
                    $$->child[0] = $1;
                    $$->child[1] = $2;
                    $$->child[2] = $3;
                }
                | termo { $$ = $1; }
                ;

soma : PLUS
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = PLUS;
        }
      | MINUS
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = MINUS;
        }
      ;

termo : termo mult fator
       {
            $$ = new_ExpNode(CalcK);
            $$->child[0] = $1;
            $$->child[1] = $2;
            $$->child[2] = $3;
       }
     | fator { $$ = $1; }
     ;

mult : TIMES
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = TIMES;
        }
      | OVER
        {
            $$ = new_ExpNode(OpK);
            $$->attr.op = OVER;
        }
      ;

fator : LPAREN expressao RPAREN { $$ = $2; }
       | var { $$ = $1; }
       | ativacao { $$ = $1; }
       | num
        {
            $$ = new_ExpNode(ConstK);
            $$->type = Integer;
            $$->attr.val = atoi(token_string);
        };
       ;

ativacao : id
            {
                $$ = new_ExpNode(CallK);
                $$->attr.name = saved_name;
            }
            LPAREN args RPAREN
            {
                $$ = $2;
                $$->child[0] = $4;
            }
            ;

args : arg_lista { $$ = $1; }
     | %empty { $$ = NULL; }
     ;

arg_lista : arg_lista COMMA expressao
            {
                YYSTYPE t = $1;

                if (t != NULL) {
                    while (t->sibling != NULL) {
                        t = t->sibling;
                    }

                    t->sibling = $3;
                    $$ = $1;
                } else {
                    $$ = $3;
                }
            }
            | expressao { $$ = $1; }
            ;

%%

/* Função yyerror retorna um erro sintático caso aconteça */
int yyerror(char *message) {
    (void)message; // Suprime os avisos de variável não usada

    fprintf(listing,"ERRO SINTÁTICO: %s LINHA: %d\n", token_string, lineno);
    exit(EXIT_FAILURE);

    return EXIT_FAILURE; // Não acontece, ver o que fazer
}

/* Função `yylex` retorna o próximo token */
static int yylex(void) { return get_token(); }

/* Função `parse` constrói a árvore sintática */
TreeNode *parse(void) {
    yyparse();

    return saved_tree;
}

