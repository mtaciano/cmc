/* Código intermediário */

#ifndef _CODE_H_
#define _CODE_H_

#include "common/globals.h"

/* Struct de Quádruplas */
typedef struct QuadRec {
    char *command;
    char *arg1;
    char *arg2;
    char *arg3;
    struct QuadRec *next;
} * Quad;

/* Função make_intermediate é responsável por gerar o código intermediário */
Quad make_intermediate(TreeNode *t);

#endif
