/* Funções da tabela de símbolos */

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "common/globals.h"

/* Função `symbol_table_insert` coloca as linhas,
 * posições de memória e os escopos na tabela de símbolos
 */
void symbol_table_insert(char *name, char *var_or_fun, char *type, char *scope,
                         int lineno, int memloc);

/* Função `symbol_table_lookup` retorna a posição na memória de
 * uma variável, e -1 se não encontrar
 */
int symbol_table_lookup(char *name);

/* Função `symbol_table_lookup_scope` retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int symbol_table_lookup_scope(char *name, char *scope);

/* Função `symbol_table_lookup_max_line` retorna o número da linha de
 * uma função, e -1 se não encontrar
 */
int symbol_table_lookup_max_line(char *var_or_fun, char *scope);

/* Função `symbol_table_print` printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void symbol_table_print(FILE *listing);

#endif /* _SYMTAB_H_ */
