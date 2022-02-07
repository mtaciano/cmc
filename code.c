/****************************************************/
/* File: code.c                                     */
/* Código intermediário                             */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include "globals.h"
#include "code.h"

#define TAM_PILHA 200
#define TAM_STRING 10

/* Valor atual da pilha, número da variável atual de temporários (_Tx)
 * e número da variável atual de endereços (_Lx)
 */
static int topoPilha = -1;
static int atualTemp = 1;
static int atualEnd = 1;

/* Pilha usada para guardar a ordem das operações */
static char pilha[TAM_PILHA][TAM_STRING];

/* Quantos CalcK existem acima */
static int calcNestedLevel = 0;

/* Função addStack adiciona o nó atual no stack */
static void addStack(TreeNode *t) {
  switch (t->nodekind) {
  case ExpK:
    switch (t->kind.exp) {
    case IdK:
      topoPilha++;
      strcpy(pilha[topoPilha], t->attr.name);
      break;

    case OpK:
      switch (t->attr.op) {
      case LT:
        topoPilha++;
        strcpy(pilha[topoPilha], "<");
        break;

      case LE:
        topoPilha++;
        strcpy(pilha[topoPilha], "<=");
        break;

      case GT:
        topoPilha++;
        strcpy(pilha[topoPilha], ">");
        break;

      case GE:
        topoPilha++;
        strcpy(pilha[topoPilha], ">=");
        break;

      case EQ:
        topoPilha++;
        strcpy(pilha[topoPilha], "==");
        break;

      case NE:
        topoPilha++;
        strcpy(pilha[topoPilha], "!=");
        break;

      case PLUS:
        topoPilha++;
        strcpy(pilha[topoPilha], "+");
        break;

      case MINUS:
        topoPilha++;
        strcpy(pilha[topoPilha], "-");
        break;

      case TIMES:
        topoPilha++;
        strcpy(pilha[topoPilha], "*");
        break;

      case OVER:
        topoPilha++;
        strcpy(pilha[topoPilha], "/");
        break;

      default:
        break;
      }
      break;

    case ConstK:
      topoPilha++;
      snprintf(pilha[topoPilha], TAM_STRING, "%d", t->attr.val);
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }
}

/* Função popStack printa os três valores do topo da pilha
 * como é representado por código de três endereços
 */
static void popStack(int n) {
  for (int i = 0; i < n; i++) {
    strcpy(pilha[topoPilha], "\0");
    topoPilha--;
  }
}

/* Função printStack remove o nó atual do stack */
static void printStack() {
  fprintf(listing, "%s %s %s\n", pilha[topoPilha - 2], pilha[topoPilha - 1],
          pilha[topoPilha]);
  //
}

/* Função preProc realiza as operações necessárias na descida */
static void preProc(TreeNode *t) {
  // fazer coisas descendo
  switch (t->nodekind) {
  case ExpK:
    switch (t->kind.exp) {
    case IdK:
    case OpK:
    case ConstK:
      addStack(t);
      break;

    case CalcK:
      calcNestedLevel++;
      break;

    default:
      break;
    }

    break;

  case DeclK:
    switch (t->kind.decl) {
    case FunK:
      fprintf(listing, "%s:\n", t->attr.name);
      break;

    default:
      break;
    }
  }
}

static void postProc(TreeNode *t) {
  switch (t->nodekind) {
  case ExpK:
    switch (t->kind.exp) {
    case CalcK:
      if (calcNestedLevel > 0) {
        fprintf(listing, "_t%-2d = %-3s %-2s %s\n", atualTemp,
                pilha[topoPilha - 2], pilha[topoPilha - 1], pilha[topoPilha]);
        popStack(3);
        // Adiciona _tx na pilha
        topoPilha++;
        snprintf(pilha[topoPilha], TAM_STRING, "_t%d", atualTemp);
        atualTemp++;
      } else if (calcNestedLevel == 0) {
        fprintf(listing, "  %-2s = _t%s", pilha[topoPilha - 1],
                pilha[topoPilha]);
        popStack(2);
      }

      /* remove os 3 elementos do topo da pilha */
      calcNestedLevel--;
      break;

    default:
      break;
    }
    break;

  case StmtK:
    switch (t->kind.stmt) {
    case AssignK:
      fprintf(listing, "  %-2s = %s\n", pilha[topoPilha - 1], pilha[topoPilha]);
      popStack(2);
      break;

      break;
    default:
      break;
    }

  case DeclK:
    switch (t->kind.decl) {
    case FunK:
      fprintf(listing, "\n");
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }
}
/* Função traverse é uma função recusiva que
 * atravessa a árvore sintática:
 * ela apilca preProc em pré-ordem e postProc em pós-ordem
 * na árvore apontada por t
 */
static void traverse(TreeNode *t, void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *)) {
  if (t != NULL) {
    preProc(t);
    for (int i = 0; i < MAXCHILDREN; i++) {
      traverse(t->child[i], preProc, postProc);
    }
    postProc(t);
    traverse(t->sibling, preProc, postProc);
  }
}

/* makeCode é responsável por gerar o código intermediário */
void makeCode(TreeNode *t) {
  traverse(t, preProc, postProc);
  //
}
