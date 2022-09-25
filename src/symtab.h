/* Funções da tabela de símbolos */

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "common/globals.h"

/* Função st_insert coloca as linhas,
 * posições de memória e os escopos na tabela de símbolos
 */
void st_insert(char *name, char *var_or_fun, char *type, char *scope,
               int lineno, int loc);

/* Função st_lookup retorna a posição na memória de
 * uma variável, e -1 se não encontrar
 */
int st_lookup(char *name);

/* Função st_lookup_scope retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int st_lookup_scope(char *name, char *scope);

/* Função st_lookup_max_line retorna o número da linha de
 * uma função, e -1 se não encontrar
 */
int st_lookup_max_line(char *var_or_fun, char *scope);

/* Função print_symbol_table printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void print_symbol_table(FILE *listing);

#endif
