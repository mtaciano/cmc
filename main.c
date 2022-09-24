/* Ponto de entrada para o compilador
 * Responsável por chamar todos os componentes dele
 */

// TODO: atualizar os comentários, NO_PARSE, NO_ANALYZE e NO_CODE
// TODO: garantir que todos os arquivos estejam padronizados
// TODO: refatorar as partes ruins dos códigos
// TODO: refatorar comentários
#include "globals.h"
#include "rust.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

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

int main(int argc, char *argv[]) {
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
            print_tree(syntaxTree);
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
        Quad q = make_code(syntaxTree);
        if (TraceCode)
            fprintf(listing, "\nGeração do código intermediário concluída.\n");

        make_output(q);
    }
#endif
#endif
#endif
    fclose(source);
    return 0;
}
