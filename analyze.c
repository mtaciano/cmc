/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int location = 0;
static int hasReturn = FALSE;

void findReturn(TreeNode *t) {
  if (t != NULL) {
    if (t->nodekind == StmtK && t->kind.stmt == ReturnK) {
      hasReturn = TRUE;
    }
  }
}

static void insertInput(void) {
  TreeNode *fun = newDeclNode(FunK);
  fun->type = Integer;

  TreeNode *type = newExpNode(TypeK);
  type->attr.type = INT;

  TreeNode *compound_stmt = newStmtNode(CompoundK);
  compound_stmt->child[0] = NULL;
  compound_stmt->child[1] = NULL;

  fun->lineno = 0;
  fun->attr.name = "input";
  fun->child[0] = type;
  fun->child[1] = NULL;
  fun->child[2] = compound_stmt;

  /* Insert input function*/
  st_insert("input", 0, location++);
}

static void insertOutput(void) {

  TreeNode *fun = newDeclNode(FunK);
  fun->type = Void;

  TreeNode *type = newDeclNode(FunK);
  type->attr.type = VOID;

  TreeNode *params = newDeclNode(ParamK);
  params->attr.name = "x";
  params->child[0] = newExpNode(TypeK);
  params->child[0]->attr.type = INT;

  TreeNode *compound_stmt = newStmtNode(CompoundK);
  compound_stmt->child[0] = NULL;
  compound_stmt->child[1] = NULL;

  fun->lineno = 0;
  fun->attr.name = "output";
  fun->child[0] = type;
  fun->child[1] = params;
  fun->child[2] = compound_stmt;

  /* Insert output function*/
  st_insert("output", 0, location++);
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
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

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode *t) {
  if (t == NULL)
    return;
  else
    return;
}

/* Função symbolError imprime um erro durante a tabela de simbolos
 * tal erro tambem é um erro semantico
 */
static void symbolError(TreeNode *t, char *message) {
  fprintf(listing, "ERRO SEMANTICO: %s LINHA: %d\n", message, t->lineno);
  Error = TRUE;
  exit(-1);
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode *t) {
  switch (t->nodekind) {
  case StmtK:
    switch (t->kind.stmt) {
    case AssignK:
      if (st_lookup(t->child[0]->attr.name) == -1) {
        if (t->child[0]->type == Void) {
          symbolError(t, "Váriável não declarada");
          break;
        }
      }
      break;

    default:
      break;
    }
    break;

  case ExpK:
    switch (t->kind.exp) {
    case IdK:
    case ArrIdK:
    case CallK: {
      if (st_lookup(t->attr.name) == -1) {
        /* not yet in table, so treat as new definition */
        st_insert(t->attr.name, t->lineno, location++);
      } else
        /* already in table, so ignore location,
           add line number of use only */
        st_insert(t->attr.name, t->lineno, 0);
      break;
    }
    default:
      break;
    }
    break;

  case DeclK:
    switch (t->kind.decl) {
    case FunK:
      /* Look up scope list to check scope existence */
      if (st_lookup(t->attr.name) != -1) {
        symbolError(t, "Redefinição de função");
        break;
      }

      if (st_lookup(t->attr.name) == -1) {
        /* not yet in table, so treat as new definition */
        st_insert(t->attr.name, t->lineno, location++);
      }
      break;
    case VarK:
      /* Look up to check variable existence */
      if (st_lookup(t->attr.name) != -1) {
        symbolError(t, "Redefinição de variável");
        break;
      }
      // Type Checing : Type should not be void
      if (t->child[0]->type == Void) {
        symbolError(t, "Variável não pode ser do tipo void");
        break;
      }

      st_insert(t->attr.name, t->lineno, location++);
      break;
    case ArrVarK:
      // Type Checing : Type should not be void
      if (t->child[0]->type == Void) {
        symbolError(t, "Variável do tipo array não pode ser void");
        break;
      }

      /*  Look up to check array variable existence  */
      if (st_lookup(t->attr.arr.name) != -1) {
        symbolError(t, "Variável do tipo array já declarada");
        break;
      }

      st_insert(t->attr.name, t->lineno, location++);
      break;
    case ArrParamK:
      if (t->attr.name == NULL) {
        break;
      }

      // Type Checing : Type should not be void
      if (t->child[0]->type == Void) {
        symbolError(t, "Parâmetro do tipo array não pode ser void");
        break;
      }

      /*  Look up to check array variable existence  */
      if (st_lookup(t->attr.name) != -1) {
        symbolError(t, "Parâmetro do tipo array já declarado");
        break;
      }

      st_insert(t->attr.name, t->lineno, location++);
      break;
    case ParamK:
      if (t->attr.name != NULL) {
        /*  Look up to check parameter existence  */
        if (t->child[0]->type == Void) {
          symbolError(t, "Parâmetro não pode ser do tipo void");
          break;
        }
        if (st_lookup(t->attr.name) != -1) {
          symbolError(t, "Redefinição de parâmetro");
          break;
        }

        st_insert(t->attr.name, t->lineno, location++);
        break;
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree) {
  insertInput();
  insertOutput();
  traverse(syntaxTree, insertNode, nullProc);
  if (TraceAnalyze) {
    fprintf(listing, "\nTabela de símbolos:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode *t, char *message) {
  fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t) {
  switch (t->nodekind) {
  case StmtK:
    switch (t->kind.stmt) {
    case AssignK: {
      // Verify the type match of two operands when assigning.
      if (st_lookup(t->child[0]->attr.name) == -1) {
        symbolError(t->child[0], "Váriavel não declarada");
      }
      break;
    }
    default:
      break;
    }
    break;
  case ExpK:
    switch (t->kind.exp) {
    case IdK:
    case ArrIdK:
    case CallK: {
      // check undeclation
      if (st_lookup(t->attr.name) == -1) {
        symbolError(t, "Símbolo não declarado");
      }
    }
    default:
      break;
    }
  case DeclK:
    switch (t->kind.decl) {
    case FunK:
      // Find if the function has return in it
      if (t->child[2] != NULL && t->child[2]->child[1] != NULL) {
        traverse(t->child[2], findReturn, nullProc);
        if (t->child[0] != NULL && t->child[0]->type == Void) {
          if (hasReturn) {
            symbolError(t, "Nenhum valor de retorno esperado");
          }
        } else if (t->child[0] != NULL && t->child[0]->type == Integer) {
          if (!hasReturn) {
            symbolError(t, "Valor de retorno esperado");
          }
        }
        hasReturn = FALSE;
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree) {
  traverse(syntaxTree, nullProc, checkNode);
}
