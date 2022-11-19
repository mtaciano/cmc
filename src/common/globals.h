/* Tipos globais
 * Protótipos usados por todo o código
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A flag `YYPARSER` impede a inclusão do arquivo `*.tab.h` nele mesmo */
#ifndef YYPARSER

/* O Yacc/Bison gera seus próprios valores para os tokens.
 * Caso outros arquivos queiram acessar tais valores, eles precisam incluir
 * o arquivo `*.tab.h` gerado durante a compilação
 */
#include "../../build/parse.tab.h"

/* A constante `ENDFILE` é implicitamente definida pelo Yacc/Bison,
 * logo não é incluída no arquivo `*.tab.h`
 */
#define ENDFILE 0

#endif /* YYPARSER */

/* A constante `MAXRESERVED` diz o número de palavras reservadas */
#define MAXRESERVED 6

/* O Yacc/Bison gera seus próprios valores int para os tokens */
typedef int TokenType;

extern FILE *source;     /* Descritor de _input_ */
extern FILE *listing;    /* Descritor de _output_ para saída padrão */
extern FILE *errlisting; /* Descritor de _output_ para erros */

extern int lineno; /* Número da linha */

/* Tipo do nó */
typedef enum { StmtK, ExpK, DeclK } NodeKind;

/* Tipo específico do nó */
typedef enum { IfK, WhileK, AssignK, CompoundK, ReturnK } StmtKind;
typedef enum { OpK, ConstK, IdK, TypeK, ArrIdK, CallK, CalcK } ExpKind;
typedef enum { VarK, FunK, ArrVarK, ArrParamK, ParamK } DeclKind;

/* Tipo da variável ou função */
typedef enum { Void, Integer, IntegerArray } ExpType;

/* Número máximo de filhos de um nó */
#define MAXCHILDREN 3

/* Atributos exclusivos de um nó do tipo _array_ */
typedef struct ArrayAttribute {
    TokenType type;
    char *name;
    int size;
} ArrAttr;

/* Informações de um nó */
typedef struct treeNode {
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind node_kind;

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

/* A variável `g_trace_scan` faz com que informações adicionais sejam printadas
 * durante a fase de _scan_
 */
extern int g_trace_scan;

/* A variável `g_trace_parse` faz com que informações adicionais sejam printadas
 * durante a fase de _parse_
 */
extern int g_trace_parse;

/* A variável `g_trace_analyze` faz com que informações adicionais sejam
 * printadas durante a fase de análise sintática
 */
extern int g_trace_analyze;

/* A variável `g_trace_code` faz com que informações adicionais sejam printadas
 * durante a fase de geração do código intermediário
 */
extern int g_trace_code;

#endif /* _GLOBALS_H_ */
