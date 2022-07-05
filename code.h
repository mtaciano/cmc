// Código intermediário

#ifndef _CODE_H_
#define _CODE_H_

/* Struct de Quadruplas */
typedef struct QuadRec {
  char *command;
  char *arg1;
  char *arg2;
  char *arg3;
  struct QuadRec *next;
} * Quad;

/* make_code é responsável por gerar o código intermediário */
#include "globals.h"
Quad make_code(TreeNode *t);

#endif
