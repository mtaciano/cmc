/****************************************************/
/* File: util.c                                     */
/* Implementação das funções utilitarias            */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Printando os tokens e lexemas */
void printToken(TokenType token, const char *tokenString) {
  switch (token) {
  case IF:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case ELSE:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case INT:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case VOID:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case RETURN:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case WHILE:
    fprintf(listing, "Palavra reservada: %s\n", tokenString);
    break;
  case PLUS:
    fprintf(listing, "+\n");
    break;
  case MINUS:
    fprintf(listing, "-\n");
    break;
  case TIMES:
    fprintf(listing, "*\n");
    break;
  case OVER:
    fprintf(listing, "/\n");
    break;
  case LT:
    fprintf(listing, "<\n");
    break;
  case LE:
    fprintf(listing, "<=\n");
    break;
  case GT:
    fprintf(listing, ">\n");
    break;
  case GE:
    fprintf(listing, ">=\n");
    break;
  case EQ:
    fprintf(listing, "==\n");
    break;
  case NE:
    fprintf(listing, "!=\n");
    break;
  case ASSIGN:
    fprintf(listing, "=\n");
    break;
  case SEMI:
    fprintf(listing, ";\n");
    break;
  case COMMA:
    fprintf(listing, ",\n");
    break;
  case LPAREN:
    fprintf(listing, "(\n");
    break;
  case RPAREN:
    fprintf(listing, ")\n");
    break;
  case LBRACKET:
    fprintf(listing, "[\n");
    break;
  case RBRACKET:
    fprintf(listing, "]\n");
    break;
  case LBRACE:
    fprintf(listing, "{\n");
    break;
  case RBRACE:
    fprintf(listing, "}\n");
    break;
  case ENDFILE:
    fprintf(listing, "%s %s\n", "ENDFILE", "EOF");
    break;
  case NUM:
    fprintf(listing, "NUM, val = %s\n", tokenString);
    break;
  case ID:
    fprintf(listing, "ID, nome = %s\n", tokenString);
    break;
  case ERROR:
    fprintf(listing, "ERRO LEXICO: %s LINHA: %d\n", tokenString, lineno);
    Error = TRUE;
    break;
  default: /* não deve acontecer normalmente */
    fprintf(listing, "Token desconhecido: %d\n", token);
  }
}

/* Função newStmtNode cria um novo nó do tipo statement
 * para a contrução da árvore sintática
 */
TreeNode *newStmtNode(StmtKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Sem memória na linha %d\n", lineno);
  else {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Função newExpNode cria um novo nó do tipo expressão
 * para a contrução da árvore sintática
 */
TreeNode *newExpNode(ExpKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Erro de falta de momória na linha %d\n", lineno);
  else {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Função newDeclNode cria um novo nó do tipo declaração
 * para a contrução da árvore sintática
 */
TreeNode *newDeclNode(DeclKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Erro de falta de momória na linha %d\n", lineno);
  else {
    for (i = 0; i < MAXCHILDREN; i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Função copyString aloca e cria uma nova
 * cópia de uma string existente
 */
char *copyString(char *s) {
  int n;
  char *t;
  if (s == NULL)
    return NULL;
  n = strlen(s) + 1;
  t = malloc(n);
  if (t == NULL)
    fprintf(listing, "Erro de falta de momória na linha %d\n", lineno);
  else
    strcpy(t, s);
  return t;
}

/* Variável indentno é usada pelo printTree para
 * guardar o número atual da indentação
 */
static indentno = 0;

/* macros para aumentar/diminuir a indentação */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indenta printando espaços */
static void printSpaces(void) {
  int i;
  for (i = 0; i < indentno; i++)
    fprintf(listing, " ");
}

/* printType printa os tipos de funções e variáveis */
void printTypes(TreeNode *tree) {
  if (tree->child[0] != NULL) {
    switch (tree->child[0]->type) {
    case Integer:
      fprintf(listing, "int");
      break;
    case Void:
      fprintf(listing, "void");
      break;
    case IntegerArray:
      fprintf(listing, "int array");
      break;
    default:
      return;
    }
  } else {
    switch (tree->type) {
    case Integer:
      fprintf(listing, "int");
      break;
    case Void:
      fprintf(listing, "void");
      break;
    case IntegerArray:
      fprintf(listing, "int array");
      break;
    default:
      return;
    }
  }
}

/* Função printTree printa a árvore sintática para o
 * arquivo listing usando indentação para indicar sub-árvores
 */
void printTree(TreeNode *tree) {
  int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();

    // IfK, WhileK, CompoundK, ReturnK
    if (tree->nodekind == StmtK) {
      switch (tree->kind.stmt) {
      case IfK:
        fprintf(listing, "If\n");
        break;
      case WhileK:
        fprintf(listing, "While\n");
        break;
      case CompoundK:
        fprintf(listing, "Declaração composta\n");
        break;
      case ReturnK:
        fprintf(listing, "Return\n");
        break;
      case AssignK:
        fprintf(listing, "Atribuição:\n");
        break;
      default:
        fprintf(listing, "Tipo de declaração desconhecida\n");
        break;
      }
    }
    // OpK, ConstK, AssignK, IdK, TypeK, ArrIdK, CallK, CalcK
    else if (tree->nodekind == ExpK) {
      if (tree->kind.exp != TypeK)
        switch (tree->kind.exp) {
        case OpK:
          fprintf(listing, "Op: ");
          printToken(tree->attr.op, "\0");
          break;
        case ConstK:
          fprintf(listing, "Const: %d\n", tree->attr.val);
          break;
        case IdK:
          fprintf(listing, "Id: %s\n", tree->attr.name);
          break;
        case TypeK:
          break;
        case ArrIdK:
          fprintf(listing, "ArrId: %s\n", tree->attr.arr.name);
          break;
        case CallK:
          fprintf(listing, "Chamada de Função: %s\n", tree->attr.name);
          break;
        case CalcK:
          fprintf(listing, "Operador: ");
          printToken(tree->child[1]->attr.op, "\0");
          break;
        default:
          fprintf(listing, "Tipo de expressão desconhecida\n");
          break;
        }
    }
    // VarK, FunK, ArrVarK, ArrParamK, ParamK
    else if (tree->nodekind == DeclK) {
      switch (tree->kind.decl) {
      case FunK:
        fprintf(listing, "Declaração de Função:  ");
        printTypes(tree);
        fprintf(listing, " %s()\n", tree->attr.name);
        break;
      case VarK:
        fprintf(listing, "Declaração de variável:  ");
        printTypes(tree);
        fprintf(listing, " %s;\n", tree->attr.name);
        break;
      case ArrVarK:
        fprintf(listing, "Declaração de array de variável:  ");
        printTypes(tree);
        fprintf(listing, " %s[%d];\n", tree->attr.arr.name, tree->attr.arr.size);
        break;
      case ArrParamK:
        fprintf(listing, "Parâmetro de array: %s\n", tree->attr.name);
        break;
      case ParamK:
        fprintf(listing, "Parâmetro: ");
        printTypes(tree);
        if (tree->attr.name != NULL) {
          fprintf(listing, " %s\n", tree->attr.name);
        } else {
          fprintf(listing, " void\n");
        }
        break;
      default:
        fprintf(listing, "Declaração desconhecida\n");
        break;
      }
    } else
      fprintf(listing, "Tipo de nó desconhecido\n");
    for (i = 0; i < MAXCHILDREN; i++)
      if (tree->child[i] != NULL)
        printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
