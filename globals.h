/* Tipos globais
 * Protótipos usados por todo o código
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A flag YYPARSER impede a inclusão do arquivo *.tab.h nele mesmo */
#ifndef YYPARSER

/* Yacc/Bison gera seus próprios valores para os tokens.
 * Caso outros arquivos queiram acessar tais valores, eles precisam incluir
 * o arquivo *.tab.h gerado durante a compilação
 */
#include "parse.tab.h"

/* ENDFILE é implicitamente definido pelo Yacc/Bison,
 * e não é incluído no arquivo *.tab.h
 */
#define ENDFILE 0

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED é o número de palavras reservadas */
#define MAXRESERVED 6

/* Yacc/Bison gera seus próprios valores int para os tokens */
typedef int TokenType;

extern FILE *source;  /* arquivo de _input_ */
extern FILE *listing; /* arquivo de _output_ para debug */

extern int lineno; /* Número da linha */

/**************************************************/
/*************   Árvore sintática   ***************/
/**************************************************/

/* Tipo do nó */
typedef enum { StmtK, ExpK, DeclK } NodeKind;
typedef enum { IfK, WhileK, AssignK, CompoundK, ReturnK } StmtKind;
typedef enum { OpK, ConstK, IdK, TypeK, ArrIdK, CallK, CalcK } ExpKind;
typedef enum { VarK, FunK, ArrVarK, ArrParamK, ParamK } DeclKind;

/* Tipo da variável ou função */
typedef enum { Void, Integer, IntegerArray } ExpType;

/* Número máximo de filhos de um nó */
#define MAXCHILDREN 3

typedef struct ArrayAttribute {
    TokenType type;
    char *name;
    int size;
} ArrAttr;

typedef struct treeNode {
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind nodekind;

    union {
        StmtKind stmt;
        ExpKind exp;
        DeclKind decl;
    } kind;

    union {
        TokenType op;
        TokenType type;
        int val;
        char *name;
        ArrAttr arr;
    } attr;

    char *scope;
    ExpType type;
} TreeNode;

/**************************************************/
/**************   Flags para Debug   **************/
/**************************************************/

/* EchoSource faz com que o arquivo de entrada seja printado
 * junto as suas linhas
 */
extern int EchoSource;

/* TraceScan faz com que informações adicionais sejam printadas
 * durante a fase de _scan_
 */
extern int TraceScan;

/* TraceParse faz com que informações adicionais sejam printadas
 * durante a fase de _parse_
 */
extern int TraceParse;

/* TraceAnalyze faz com que informações adicionais sejam printadas
 * durante a fase de análise sintática
 */
extern int TraceAnalyze;

/* TraceCode faz com que informações adicionais sejam printadas
 * durante a fase de geração do código intermediário
 */
extern int TraceCode;

/* Error faz com que a compilação pare */
extern int Error;

#endif
