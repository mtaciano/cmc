/* Funções para o analisador semântico */

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "common/globals.h"

/* Função `build_symtab` constrói a tabela de símbolos
 * através de uma passada em pré ordem da árvore sintática
 */
void build_symtab(TreeNode *syntax_tree);

/* Função `type_check realiza` verificação de tipos
 * através de uma travessia em pós-ordem da árvore síntatica
 */
void type_check(TreeNode *syntax_tree);

#endif /* _ANALYZE_H_ */
