/****************************************************/
/* File: symtab.h                                   */
/* Funções da tabela de símbolos                    */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* Função st_insert coloca as linhas,
 * posições de memória e os escopos na tabela de símbolos
 */
void st_insert(char *name, char *varFun, char *tipo, char *scope, int lineno, int loc);

/* Função st_lookup retorna a posição na memória de
 * uma variável, e -1 se não encontrar
 */
int st_lookup(char *name);

/* Função st_lookup_scope retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int st_lookup_scope(char *name, char *scope);

/* Função st_lookup_max_linha retorna o número da linha de
 * uma função, e -1 se não encontrar
 */
int st_lookup_max_linha(char *varFun, char *scope);

/* Função printSymTab printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void printSymTab(FILE *listing);

#endif
