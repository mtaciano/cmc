/* Ponto de entrada para o compilador
 * Responsável por chamar todos os componentes dele
 */
#include <stdbool.h>

#include "common/globals.h"
#include "common/util.h"
#include "rust.h"

/* Mude NO_PARSE para true para ter um compilador _scanner-only_ */
#define NO_PARSE false

/* Mude NO_ANALYZE para true para ter um compilador _parser-only_ */
#define NO_ANALYZE false

/* Mude NO_CODE para true para ter um compilador que não gera _output_ */
#define NO_CODE false

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
int g_mem_start;
int g_mem_end;
int g_inst_start;
int g_inst_end;
FILE *source;
FILE *std_fd;
FILE *err_fd;

/* Flags para debug */
int g_trace_scan = false;
int g_trace_parse = false;
int g_trace_analyze = false;
int g_trace_code = true;

/* Função principal */
int
main(int argc, char *argv[])
{
    TreeNode *syntax_tree;
    char *program; /* Nome do arquivo de entrada */

    std_fd = stdout; /* Manda o texto para STDOUT */
    err_fd = stderr; /* Manda o texto para STDERR */

    if (argc != 3) {
        fprintf(err_fd, "uso: %s <nome_arquivo> <slot>\n", argv[0]);
        fprintf(err_fd, "slots:\n");
        fprintf(err_fd, "  0 -> sistema operacional\n");
        fprintf(err_fd, "  1 -> programa 1\n");
        fprintf(err_fd, "  2 -> programa 2\n");
        fprintf(err_fd, "  3 -> programa 3\n");
        exit(EXIT_FAILURE);
    }

    program = malloc_or_die(strlen(argv[1]) + 1);
    strcpy(program, argv[1]);

    if (strchr(program, '.') == NULL) {
        // Caso o arquivo não tenha extensão, adicionar `.cm`
        strcat(program, ".cm");
    }

    source = fopen(program, "r");

    if (source == NULL) {
        fprintf(err_fd, "Arquivo %s não encontrado.\n", program);
        exit(EXIT_FAILURE);
    }

    char *errptr;
    int slot = strtol(argv[2], &errptr, 10);
    if (errptr == argv[2] || (slot < 0 || slot >= 4)) {
        fprintf(err_fd, "slot '%s' não pode ser usado.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    g_mem_start = slot * MEM_AVAILABLE;
    g_mem_end = g_mem_start + MEM_AVAILABLE;

    g_inst_start = slot * INST_AVAILABLE;
    g_inst_end = g_inst_start + INST_AVAILABLE;

    fprintf(
        std_fd, "COMPILAÇÃO DO C- (%d-%d): %s\n\n", g_inst_start, g_inst_end,
        program
    );

#if NO_PARSE
    while (get_token() != ENDFILE) {
        ;
    }
#else
    syntax_tree = parse();

    if (g_trace_parse) {
        fprintf(std_fd, "\nÁrvore sintática:\n\n");
        print_tree(syntax_tree);
    }
#if !NO_ANALYZE
    if (g_trace_analyze) {
        fprintf(std_fd, "\nMontando tabela de símbolos...\n");
    }

    build_symtab(syntax_tree);

    if (g_trace_analyze) {
        fprintf(std_fd, "\nVerificando tipos...\n");
    }

    type_check(syntax_tree);

    if (g_trace_analyze) {
        fprintf(std_fd, "\nVerificação concluída.\n");
    }
#if !NO_CODE
    if (g_trace_code) {
        fprintf(std_fd, "\nGerando código intermediário...\n\n");
    }

    Quad quadruple = make_intermediate(syntax_tree);

    if (g_trace_code) {
        fprintf(std_fd, "\nGeração do código intermediário concluída.\n");
    }

    make_assembly_and_binary(quadruple);
#endif
#endif
#endif
    fclose(source);
    free(program);

    return 0;
}
