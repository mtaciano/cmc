/****************************************************/
/* File: code.c                                     */
/* Código intermediário                             */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include "globals.h"
#include "code.h"

/* Struct de Quadruplas */
typedef struct QuadRec {
  char *command;
  char *arg1;
  char *arg2;
  char *arg3;
  struct QuadRec *next;
} * Quad;

/* Quadrupla final */
static Quad quad;

/* Função insert_quad insere quadruplas na variável quad */
static Quad insert_quad(char *cmd, char *arg1, char *arg2, char *arg3) {
  if (quad == NULL) {
    quad = (Quad)malloc(sizeof(struct QuadRec));
    quad->command = cmd;
    quad->arg1 = arg1;
    quad->arg2 = arg2;
    quad->arg3 = arg3;
    quad->next = NULL;
  } else {
    Quad new_quad = (Quad)malloc(sizeof(struct QuadRec));
    new_quad->command = cmd;
    new_quad->arg1 = arg1;
    new_quad->arg2 = arg2;
    new_quad->arg3 = arg3;
    new_quad->next = NULL;

    Quad last_quad = quad;
    while (last_quad->next != NULL) {
      last_quad = last_quad->next;
    }

    last_quad->next = new_quad;
  }
}

/* Função type_to_string transforma o tipo em uma string, void ou int */
char *type_to_string(TreeNode *t) {
  switch (t->type) {
  case Integer:
    return "INT";
    break;
  case Void:
    return "VOID";
    break;
  }
}

/* Função read_tree_node lê o nó da arvore e trata ele de acordo */
/* TODO: tratar todos os casos possíveis */
static void read_tree_node(TreeNode *t) {
  // Provavelmente vai ter variaveis aqui

  // Verifica o nó atual
  switch (t->nodekind) {
  case StmtK:
    switch (t->kind.stmt) {
    case IfK:
      /* code */
      break;
    case WhileK:
      /* code */
      break;
    case AssignK:
      /* code */
      break;
    case CompoundK:
      /* code */
      break;
    case ReturnK:
      /* code */
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp) {
    case OpK:
      /* code */
      break;
    case ConstK:
      /* code */
      break;
    case IdK:
      /* code */
      break;
    case TypeK:
      /* code */
      break;
    case ArrIdK:
      /* code */
      break;
    case CallK:
      /* code */
      break;
    case CalcK:
      /* code */
      break;
    }
    break;
  case DeclK:
    switch (t->kind.decl) {
    case VarK:
      /* code */
      break;
    case FunK:
      char *type = type_to_string(t->child[0]);
      char *name = t->attr.name;
      insert_quad("FUN", type, name, "--");
      break;
    case ArrVarK:
      /* code */
      break;
    case ArrParamK:
      /* code */
      break;
    case ParamK:
      /* code */
      break;
    }
    break;
  }

  // Verifica os 3 filhos
  for (int i = 1; i < 3; i++) {
    if (t->child[i] != NULL) {
      read_tree_node(t->child[i]);
    }
  }

  // Verifica o irmão
  if (t->sibling != NULL) {
    read_tree_node(t->sibling);
  }
}

/* makeCode é responsável por gerar o código intermediário */
/* TODO: transformar os nomes das funções e variáveis
 *   @t! de Camel case para Snake case
 */
void makeCode(TreeNode *t) {

  read_tree_node(t);
  //
}
