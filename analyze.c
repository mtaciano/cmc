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

/* counter for variable memory locations */
static int location = 0;

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
      if (t->child[0]->attr.arr.type == IntegerArray) {
        typeError(t->child[0], "Assignment to Integer Array Variable");
      }

      if (t->child[0]->attr.type == Void) {
        typeError(t->child[0], "Assignment to Void Variable");
      }
      break;
    }
    case ReturnK: {
      const TreeNode *funcDecl;
      const ExpType funcType = funcDecl->type;
      const TreeNode *expr = t->child[0];

      if (funcType == Void && (expr != NULL && expr->type != Void)) {
        typeError(t, "expected no return value");
      } else if (funcType == Integer && (expr == NULL || expr->type == Void)) {
        typeError(t, "expected return value");
      }
    }
    default:
      break;
    }
    break;
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
