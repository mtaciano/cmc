/****************************************************/
/* Arquivo: util.h                                  */
/* Funções Utilitarias para  o compilador de C-     */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

/* Função printToken printa um token
 * e seu lexema para o arquivo listing
 */
void printToken(TokenType, const char *);

/* Função newStmtNode cria um novo nó do tipo statement
 * para a contrução da árvore sintática
 */
TreeNode *newStmtNode(StmtKind);

/* Função newExpNode cria um novo nó do tipo expressão
 * para a contrução da árvore sintática
 */
TreeNode *newExpNode(ExpKind);

/* Função newDeclNode cria um novo nó do tipo declaração
 * para a contrução da árvore sintática
 */
TreeNode *newDeclNode(DeclKind);

/* Função copyString aloca e cria uma nova
 * cópia de uma string existente
 */
char *copyString(char *);

/* Função printTree printa a árvore sintática para o
 * arquivo listing usando indentação para indicar sub-árvores
 */
void printTree(TreeNode *);

#endif
