/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE TRUE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "code.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

main(int argc, char *argv[]) {
  TreeNode *syntaxTree;
  char pgm[120]; /* source code file name */
  if (argc != 2) {
    fprintf(stderr, "uso: %s <nome_arquivo>\n", argv[0]);
    exit(1);
  }
  strcpy(pgm, argv[1]);
  if (strchr(pgm, '.') == NULL)
    strcat(pgm, ".cm");
  source = fopen(pgm, "r");
  if (source == NULL) {
    fprintf(stderr, "Arquivo %s não encontrado\n", pgm);
    exit(1);
  }
  listing = stdout; /* send listing to screen */
  fprintf(listing, "\nCOMPILAÇÃO DO C-: %s\n\n", pgm);
#if NO_PARSE
  while (getToken() != ENDFILE)
    ;
#else
  syntaxTree = parse();
  if (!Error) {
    if (TraceParse) {
      fprintf(listing, "\nArvore sintática:\n\n");
      printTree(syntaxTree);
    }
  }
#if !NO_ANALYZE
  if (!Error) {
    if (TraceAnalyze)
      fprintf(listing, "\nMontando tabela de símbolos...\n");
    buildSymtab(syntaxTree);
    if (TraceAnalyze)
      fprintf(listing, "\nVerificando tipos...\n");
    typeCheck(syntaxTree);
    if (TraceAnalyze)
      fprintf(listing, "\nVerificação concluída.\n");
  }
#if !NO_CODE
  if (!Error) {
    if (TraceCode)
      fprintf(listing, "\nGerando código intermediário\n\n");
    makeCode(syntaxTree);
    if (TraceCode)
      fprintf(listing, "\nGeração do código intermediário concluída.\n");
  }
#endif
#endif
#endif
  fclose(source);
  return 0;
}
