/* Funções para o analisador semântico */

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "globals.h"

/* Função build_symbol_table constrói uma tabela de símbolos
 * por travessia em pré-ordem da árvore síntatica
 */
void build_symbol_table(TreeNode *syntax_tree);

/* Função type_check realiza verificação de tipos
 * através de uma travessia em pós-ordem da árvore síntatica
 */
void type_check(TreeNode *syntax_tree);

#endif
