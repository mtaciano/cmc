/* Ponto de entrada para o compilador
 * Responsável por chamar todos os componentes dele
 */

#include "common/globals.h"
#include "common/util.h"
#include "rust.h"

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
#include "intermediate.h"
#endif
#endif
#endif

/* Variáveis globais */
int lineno = 0;
FILE *source;
FILE *listing;

/* Flags para debug */
int g_trace_scan = TRUE;
int g_trace_parse = TRUE;
int g_trace_analyze = TRUE;
int g_trace_code = TRUE;

/* Flag de erro de compilação */
int g_error = FALSE;

/* Função principal */
int main(int argc, char *argv[]) {
    TreeNode *syntax_tree;
    char *program; /* Nome do arquivo de entrada */

    if (argc != 2) {
        fprintf(stderr, "uso: ./%s <nome_arquivo>\n", argv[0]);
        exit(-1);
    }

    program = malloc(strlen(argv[1]) * sizeof(argv[1]));
    strcpy(program, argv[1]);

    if (strchr(program, '.') == NULL) {
        // Caso o arquivo não tenha extensão, adicionar `.cm`
        strcat(program, ".cm");
    }

    source = fopen(program, "r");

    if (source == NULL) {
        fprintf(stderr, "Arquivo %s não encontrado\n", program);
        exit(-1);
    }

    listing = stdout; /* Manda o texto para STDOUT */
    fprintf(listing, "\nCOMPILAÇÃO DO C-: %s\n\n", program);

#if NO_PARSE
    while (get_token() != ENDFILE)
        ;
#else
    syntax_tree = parse();

    if (!g_error) {
        if (g_trace_parse) {
            fprintf(listing, "\nArvore sintática:\n\n");
            print_tree(syntax_tree);
        }
    }
#if !NO_ANALYZE
    if (!g_error) {
        if (g_trace_analyze) {
            fprintf(listing, "\nMontando tabela de símbolos...\n");
        }

        build_symbol_table(syntax_tree);

        if (g_trace_analyze) {
            fprintf(listing, "\nVerificando tipos...\n");
        }

        type_check(syntax_tree);

        if (g_trace_analyze) {
            fprintf(listing, "\nVerificação concluída.\n");
        }
    }
#if !NO_CODE
    if (!g_error) {
        if (g_trace_code) {
            fprintf(listing, "\nGerando código intermediário\n\n");
        }

        Quad quad = make_intermediate(syntax_tree);

        if (g_trace_code) {
            fprintf(listing, "\nGeração do código intermediário concluída.\n");
        }

        make_assembly_and_binary(quad);
    }
#endif
#endif
#endif
    fclose(source);
    free(program);

    return 0;
}
