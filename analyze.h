// Funções para o analisador semântico

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/* Função buildSymtab constrói uma tabela de símbolos
 * por travessia em pré-ordem da árvore síntatica
 */
void buildSymtab(TreeNode *);

/* Função typeCheck realiza verificação de tipos
 * através de uma travessia em pós-ordem da árvore síntatica
 */
void typeCheck(TreeNode *);

#endif
