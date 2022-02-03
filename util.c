/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString) {
  switch (token) {
  case IF:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;
  case ELSE:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;
  case INT:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;
  case VOID:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;
  case RETURN:
    fprintf(listing, "reserved word: %s\n", tokenString);
    break;
  case WHILE:
    fprintf(listing, "reserved word: %s\n", tokenString);
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
    fprintf(listing, "ID, name = %s\n", tokenString);
    break;
  case ERROR:
    fprintf(listing, "ERROR: %s\n", tokenString);
    break;
  default: /* should never happen */
    fprintf(listing, "Unknown token: %d\n", token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newStmtNode(StmtKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
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

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode *newExpNode(ExpKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
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

/* Function newDeclNode creates a new declaration
 * node for syntax tree construction
 */
TreeNode *newDeclNode(DeclKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
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

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s) {
  int n;
  char *t;
  if (s == NULL)
    return NULL;
  n = strlen(s) + 1;
  t = malloc(n);
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
    strcpy(t, s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
  int i;
  for (i = 0; i < indentno; i++)
    fprintf(listing, " ");
}

/* printType print types of funcions and variables */
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

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
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
        fprintf(listing, "Compound statement\n");
        break;
      case ReturnK:
        fprintf(listing, "Return\n");
        break;
      case AssignK:
        fprintf(listing, "Assign:\n");
        break;
      default:
        fprintf(listing, "Unknown StmtNode kind\n");
        break;
      }
    }
    // OpK, ConstK, AssignK, IdK, TypeK, ArrIdK, CallK, CalK
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
          fprintf(listing, "Call Function: %s\n", tree->attr.name);
          break;
        case CalcK:
          fprintf(listing, "Operator: ");
          printToken(tree->child[1]->attr.op, "\0");
          break;
        default:
          fprintf(listing, "Unknown ExpNode kind\n");
          break;
        }
    }
    // VarK, FunK, ArrVarK, ArrParamK, ParamK
    else if (tree->nodekind == DeclK) {
      switch (tree->kind.decl) {
      case FunK:
        fprintf(listing, "Function Declaration:  ");
        printTypes(tree);
        fprintf(listing, " %s()\n", tree->attr.name);
        break;
      case VarK:
        fprintf(listing, "Variable Declaration:  ");
        printTypes(tree);
        fprintf(listing, " %s;\n", tree->attr.name);
        break;
      case ArrVarK:
        fprintf(listing, "Array Variable Declaration:  ");
        printTypes(tree);
        fprintf(listing, " %s[%d];\n", tree->attr.arr.name,
                tree->attr.arr.size);
        break;
      case ArrParamK:
        fprintf(listing, "Array Parameter: %s\n", tree->attr.name);
        break;
      case ParamK:
        fprintf(listing, "Parameter: ");
        printTypes(tree);
        if (tree->attr.name != NULL) {
          fprintf(listing, " %s\n", tree->attr.name);
        } else {
          fprintf(listing, " void\n");
        }
        break;
      default:
        fprintf(listing, "Unknown Declaration\n");
        break;
      }
    } else
      fprintf(listing, "Unknown node kind\n");
    for (i = 0; i < MAXCHILDREN; i++)
      if (tree->child[i] != NULL)
        printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
