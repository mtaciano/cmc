/* Funções da tabela de símbolos */

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "common/globals.h"

/* Função `symtab_insert` coloca as linhas,
 * posições de memória e os escopos na tabela de símbolos
 */
void symtab_insert(
    char *name, char *varfn, char *type, char *scope, int lineno, int memloc
);

/* Função `symtab_lookup` retorna a posição na memória de
 * uma variável, e -1 se não encontrar
 */
int symtab_lookup(char *name);

/* Função `symtab_lookup_scope` retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int symtab_lookup_scope(char *name, char *scope);

/* Função `symtab_lookup_max_line` retorna o número da linha de
 * uma função, e -1 se não encontrar
 */
int symtab_lookup_max_line(char *varfn, char *scope);

/* Função `symtab_print` printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void symtab_print(FILE *listing);

#endif /* _SYMTAB_H_ */
