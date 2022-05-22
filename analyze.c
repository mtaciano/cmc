/****************************************************/
/* File: analyze.c                                  */
/* Analize Semântica da linguagem C-                */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

#define TAM_PILHA 100
#define TAM_STRING 100

/* contador da posição na memória */
static int location = 0;
static int hasReturn = FALSE;
static int insideParam = FALSE;

/* componentes da pilha */
static int topoPilha = 0;
static char pilha[TAM_PILHA][TAM_STRING];

/* Função symbolError imprime um erro durante a tabela de simbolos
 * tal erro tambem é um erro semantico
 */
static void symbolError(TreeNode *t, char *message) {
  fprintf(listing, "ERRO SEMANTICO: %s LINHA: %d\n", message, t->lineno);
  Error = TRUE;
  exit(-1);
}

static void verificaMain(TreeNode *t) {
  int linhaMax = st_lookup_max_linha("fun", "global");
  int varLinhaMax = st_lookup_max_linha("var", "global");
  int linhaMain = st_lookup("main");
  if (linhaMax > linhaMain || varLinhaMax > linhaMain) {
    int max = linhaMax > varLinhaMax ? linhaMax : varLinhaMax;
    fprintf(listing, "ERRO SEMANTICO: declaração depois do main LINHA: %d\n",
            max);
    Error = TRUE;
    exit(-1);
  }
}

/* findReturn() verifica se a função possui retorno */
static void findReturn(TreeNode *t) {
  if (t != NULL) {
    if (t->nodekind == StmtK && t->kind.stmt == ReturnK) {
      hasReturn = TRUE;
    }
  }
}

/* giveScope() recursivamente cria escopo para os nós */
static void giveScope(TreeNode *t) {
  if (t != NULL) {
    t->scope = (char *)malloc(TAM_STRING);
    if (t->nodekind == DeclK && t->kind.decl == FunK) {
      strcpy(t->scope, pilha[topoPilha]);
      topoPilha++;
      strcpy(pilha[topoPilha], t->attr.name);
    } else {
      strcpy(t->scope, pilha[topoPilha]);
    }
  }
}

static void popStack(TreeNode *t) {
  if (t != NULL) {
    if (t->nodekind == DeclK && t->kind.decl == FunK) {
      strcpy(pilha[topoPilha], "\0");
      topoPilha--;
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

  /* Coloca o insert */
  st_insert("input", "fun", "int", "global", 0, location++);
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

  /* Colocar o output */
  st_insert("output", "fun", "void", "global", 0, location++);
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

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode *t) {
  char *varFun;
  char *tipo;

  switch (t->nodekind) {
  case StmtK:
    switch (t->kind.stmt) {
    case AssignK:
      if (st_lookup_scope(t->child[0]->attr.name, t->scope) == -1 &&
          st_lookup_scope(t->child[0]->attr.name, "global") == -1) {
        symbolError(t, "Váriável não declarada");
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
      varFun = "var";
      tipo = "int";
    case CallK: {
      if (t->kind.exp == CallK) {
        varFun = "fun";
        if (t->child[0] != NULL && t->child[0]->type == Void) {
          tipo = "void";
        } else if (t->child[0] != NULL && t->child[0]->type == Integer) {
          tipo = "int";
        }
      }

      if (st_lookup(t->attr.name) == -1) {
        /* não está na tabela, nova definição */
        st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, location++);
      } else
        /* já na tabela */
        st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, 0);
      break;
    }
    default:
      break;
    }
    break;

  case DeclK:
    switch (t->kind.decl) {
    case FunK:
      if (st_lookup(t->attr.name) != -1) {
        symbolError(t, "Redefinição de função");
        break;
      }

      varFun = "fun";
      if (t->child[0] != NULL && t->child[0]->type == Void) {
        tipo = "void";
      } else if (t->child[0] != NULL && t->child[0]->type == Integer) {
        tipo = "int";
      }

      if (st_lookup(t->attr.name) == -1) {
        /* não está na tabela, nova definição */
        st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, location++);
      }
      break;
    case VarK:
      varFun = "var";
      tipo = "int";

      /* verificar se a variavel já existe */
      if (st_lookup_scope(t->attr.name, t->scope) != -1) {
        symbolError(t, "Redefinição de variável");
        break;
      }
      // tipo não deve ser VOID
      if (t->child[0]->type == Void) {
        symbolError(t, "Variável não pode ser do tipo void");
        break;
      }

      st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, location++);
      break;
    case ArrVarK:
      varFun = "var";
      tipo = "int";
      // tipo não deve ser VOID
      if (t->child[0]->type == Void) {
        symbolError(t, "Variável do tipo array não pode ser void");
        break;
      }

      /*  verificar se a variável  array já foi declarada  */
      if (st_lookup_scope(t->attr.arr.name, t->scope) != -1) {
        symbolError(t, "Variável do tipo array já declarada");
        break;
      }

      st_insert(t->attr.arr.name, varFun, tipo, t->scope, t->lineno,
                location++);
      break;
    case ArrParamK:
      if (t->attr.name == NULL) {
        break;
      }

      varFun = "var";
      tipo = "int";

      /* tipo não deve ser VOID */
      if (t->child[0]->type == Void) {
        symbolError(t, "Parâmetro do tipo array não pode ser void");
        break;
      }

      /* Verifica se o array já foi declarado */
      if (st_lookup_scope(t->attr.name, t->scope) != -1) {
        symbolError(t, "Parâmetro do tipo array já declarado");
        break;
      }

      st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, location++);
      break;
    case ParamK:
      if (t->attr.name != NULL) {
        /* Verifica se o parâmetro existe ou é void */
        if (t->child[0]->type == Void) {
          symbolError(t, "Parâmetro não pode ser do tipo void");
          break;
        }
        if (st_lookup_scope(t->attr.name, t->scope) != -1) {
          symbolError(t, "Redefinição de parâmetro");
          break;
        }

        varFun = "var";
        tipo = "int";
        st_insert(t->attr.name, varFun, tipo, t->scope, t->lineno, location++);
        break;
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
  varFun = "\0";
  tipo = "\0";
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *syntaxTree) {
  insertInput();
  insertOutput();
  strcpy(pilha[0], "global");
  traverse(syntaxTree, giveScope, popStack);
  traverse(syntaxTree, insertNode, nullProc);
  verificaMain(syntaxTree);
  if (!Error && TraceAnalyze) {
    fprintf(listing, "\nTabela de símbolos:\n\n");
    printSymTab(listing);
  }
}

/* Função typeCheck realiza verificação de tipos
 * através de uma travessia em pós-ordem da árvore síntatica
 */
static void checkNode(TreeNode *t) {
  switch (t->nodekind) {
  case StmtK:
    switch (t->kind.stmt) {
    case AssignK: {
      if (st_lookup_scope(t->child[0]->attr.name, t->scope) == -1 &&
          st_lookup_scope(t->child[0]->attr.name, "global") == -1) {
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
      // Verifica se a função tem um return se ela precisa de um
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
