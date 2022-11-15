/* Funções utilitarias para o compilador de C- */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

/* Função `printToken` printa um token
 * e seu lexema para o arquivo listing
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
 * arquivo listing usando indentação para indicar sub-árvores
 */
void print_tree(TreeNode *tree);

/* Componentes da pilha */
typedef struct {
    int max_size;
    int last;
    char **items;
} * CharStack;

/* Função `cs_init` inicia a pilha com um tamanho máximo de `size` */
CharStack cs_init(void);

/* Função `cs_push` coloca a string na pilha */
void cs_push(CharStack stack, char *item);

/* Função `cs_pop` remove a string da pilha, retornando seu valor */
char *cs_pop(CharStack stack);

/* Função `cs_drop` remove a memória usada pela pilha */
void cs_drop(CharStack stack);

#endif /* _UTIL_H_ */
