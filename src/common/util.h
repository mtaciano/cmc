/* Funções utilitarias para o compilador de C- */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

/* Função `malloc_or_die` é um _wrapper_ para a função `malloc`
 * com a diferença que, quando não foi possível realizar o `malloc`,
 * ela chama `exit` e encerra a execução do código
 */
void *malloc_or_die(size_t size);

/* Função `realloc_or_die` é um _wrapper_ para a função `realloc`
 * com a diferença que, quando não foi possível realizar o `realloc`,
 * ela chama `exit` e encerra a execução do código
 */
void *realloc_or_die(void *ptr, size_t size);

/* Função `printToken` printa um token
 * e seu lexema para o arquivo std_fd
 */
void print_token(TokenType token, const char *token_string);

/* Função `new_StmtNode` cria um novo nó do tipo statement
 * para a contrução da árvore sintática
 */
TreeNode *new_StmtNode(StmtKind kind);

/* Função `new_ExpNode` cria um novo nó do tipo expressão
 * para a contrução da árvore sintática
 */
TreeNode *new_ExpNode(ExpKind kind);

/* Função `new_DeclNode` cria um novo nó do tipo declaração
 * para a contrução da árvore sintática
 */
TreeNode *new_DeclNode(DeclKind kind);

/* Função `copy_string` aloca e cria uma nova
 * cópia de uma string existente
 */
char *copy_string(char *s);

/* Função `print_tree` printa a árvore sintática para o
 * arquivo std_fd usando indentação para indicar sub-árvores
 */
void print_tree(TreeNode *tree);

/* Pilha de strings */
typedef struct {
    int max_size;
    int last;
    char **items;
} * CharStack;

/* Função `cs_init` inicia a pilha com um tamanho máximo de `size` */
CharStack cs_init(void);

/* Função `cs_push` coloca um elemento na pilha */
void cs_push(CharStack stack, char *item);

/* Função `cs_pop` remove o elemento da pilha, retornando seu valor */
char *cs_pop(CharStack stack);

/* Função `cs_drop` remove a memória usada pela pilha */
void cs_drop(CharStack stack);

/* Função `cs_peek` retorna o elemento mais recente sem removê-lo da pilha */
char *cs_peek(CharStack stack);

#endif /* _UTIL_H_ */
