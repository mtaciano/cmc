/* Ponto de entrada para o compilador
 * Responsável por chamar todos os componentes dele
 */

#include "globals.h"
#include "rust.h"
#include "util.h"

/* Mude NO_PARSE para TRUE para ter um compilador _scanner-only_ */
#define NO_PARSE FALSE

/* Mude NO_ANALYZE para TRUE para ter um compilador _parser-only_ */
#define NO_ANALYZE FALSE

/* Mude NO_CODE para TRUE para ter um compilador que não gera _output_ */
#define NO_CODE FALSE

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

/* Variáveis globais */
int lineno = 0;
FILE *source;
FILE *listing;

/* Flags para debug */
int EchoSource = FALSE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

/* Flag de erro de compilação */
int Error = FALSE;

/* Função principal */
int main(int argc, char *argv[]) {
    TreeNode *syntaxTree;
    char pgm[120]; /* Nome do arquivo de entrada */

    if (argc != 2) {
        fprintf(stderr, "uso: %s <nome_arquivo>\n", argv[0]);
        exit(-1);
    }

    strcpy(pgm, argv[1]);

    if (strchr(pgm, '.') == NULL) {
        // Caso o arquivo não tenha extensão, adicionar `.cm`
        strcat(pgm, ".cm");
    }

    source = fopen(pgm, "r");

    if (source == NULL) {
        fprintf(stderr, "Arquivo %s não encontrado\n", pgm);
        exit(1);
    }

    listing = stdout; /* Manda o texto para STDOUT */
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
        if (TraceAnalyze) {
            fprintf(listing, "\nMontando tabela de símbolos...\n");
        }

        build_symbol_table(syntaxTree);

        if (TraceAnalyze) {
            fprintf(listing, "\nVerificando tipos...\n");
        }

        type_check(syntaxTree);

        if (TraceAnalyze) {
            fprintf(listing, "\nVerificação concluída.\n");
        }
    }
#if !NO_CODE
    if (!Error) {
        if (TraceCode) {
            fprintf(listing, "\nGerando código intermediário\n\n");
        }

        Quad q = make_code(syntaxTree);

        if (TraceCode) {
            fprintf(listing, "\nGeração do código intermediário concluída.\n");
        }

        make_output(q);
    }
#endif
#endif
#endif
    fclose(source);

    return 0;
}
