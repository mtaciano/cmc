/* Tipos globais
 * Protótipos usados por todo o código
 */

// TODO: comentar o código
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Yacc/Bison generates internally its own values
 * for the tokens. Other files can access these values
 * by including the tab.h file generated using the
 * Yacc/Bison option -d ("generate header")
 *
 * The YYPARSER flag prevents inclusion of the tab.h
 * into the Yacc/Bison output itself
 */

#ifndef YYPARSER

/* the name of the following file may change */
#include "cmin.tab.h"

/* ENDFILE is implicitly defined by Yacc/Bison,
 * and not included in the tab.h file
 */
#define ENDFILE 0

#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

/* Yacc/Bison generates its own integer values
 * for tokens
 */
typedef int TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code;    /* code text file for TM simulator */

extern int TraceCode;

extern int Error;

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK, DeclK } NodeKind;
typedef enum { IfK, WhileK, AssignK, CompoundK, ReturnK } StmtKind;
typedef enum { OpK, ConstK, IdK, TypeK, ArrIdK, CallK, CalcK } ExpKind;
typedef enum { VarK, FunK, ArrVarK, ArrParamK, ParamK } DeclKind;

/* ExpType is used for type checking */
typedef enum { Void, Integer, IntegerArray } ExpType;

#define MAXCHILDREN 3

struct ScopeListRec;

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
    ExpType type; /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
