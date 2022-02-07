/****************************************************/
/* File: parse.c                                    */
/* Implementação do parse para a linguagem C-       */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include "globals.h"
#include "parse.h"
#include "scan.h"
#include "util.h"

static TokenType token; /* token atual */

/* funções para chamadas recursivas */
static TreeNode *stmt_sequence(void);
static TreeNode *statement(void);
static TreeNode *if_stmt(void);
static TreeNode *repeat_stmt(void);
static TreeNode *assign_stmt(void);
static TreeNode *read_stmt(void);
static TreeNode *write_stmt(void);
static TreeNode *exp(void);
static TreeNode *simple_exp(void);
static TreeNode *term(void);
static TreeNode *factor(void);

static void syntaxError(char *message) {
  fprintf(listing, "\n>>> ");
  fprintf(listing, "ERRO SINTÁTICO: %s LINHA %d", message, lineno);
  Error = TRUE;
}

static void match(TokenType expected) {
  if (token == expected)
    token = getToken();
  else {
    syntaxError("token não esperado -> ");
    printToken(token, tokenString);
    fprintf(listing, "      ");
  }
}

TreeNode *stmt_sequence(void) {
  TreeNode *t = statement();
  TreeNode *p = t;
  while ((token != ENDFILE) && (token != END) && (token != ELSE) &&
         (token != UNTIL)) {
    TreeNode *q;
    match(SEMI);
    q = statement();
    if (q != NULL) {
      if (t == NULL)
        t = p = q;
      else {
        p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

TreeNode *statement(void) {
  TreeNode *t = NULL;
  switch (token) {
  case IF:
    t = if_stmt();
    break;
  case REPEAT:
    t = repeat_stmt();
    break;
  case ID:
    t = assign_stmt();
    break;
  case READ:
    t = read_stmt();
    break;
  case WRITE:
    t = write_stmt();
    break;
  default:
    syntaxError("token não esperado -> ");
    printToken(token, tokenString);
    token = getToken();
    break;
  }
  return t;
}

TreeNode *if_stmt(void) {
  TreeNode *t = newStmtNode(IfK);
  match(IF);
  if (t != NULL)
    t->child[0] = exp();
  match(THEN);
  if (t != NULL)
    t->child[1] = stmt_sequence();
  if (token == ELSE) {
    match(ELSE);
    if (t != NULL)
      t->child[2] = stmt_sequence();
  }
  match(END);
  return t;
}

TreeNode *repeat_stmt(void) {
  TreeNode *t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t != NULL)
    t->child[0] = stmt_sequence();
  match(UNTIL);
  if (t != NULL)
    t->child[1] = exp();
  return t;
}

TreeNode *assign_stmt(void) {
  TreeNode *t = newStmtNode(AssignK);
  if ((t != NULL) && (token == ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  match(ASSIGN);
  if (t != NULL)
    t->child[0] = exp();
  return t;
}

TreeNode *read_stmt(void) {
  TreeNode *t = newStmtNode(ReadK);
  match(READ);
  if ((t != NULL) && (token == ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}

TreeNode *write_stmt(void) {
  TreeNode *t = newStmtNode(WriteK);
  match(WRITE);
  if (t != NULL)
    t->child[0] = exp();
  return t;
}

TreeNode *exp(void) {
  TreeNode *t = simple_exp();
  if ((token == LT) || (token == EQ)) {
    TreeNode *p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
    }
    match(token);
    if (t != NULL)
      t->child[1] = simple_exp();
  }
  return t;
}

TreeNode *simple_exp(void) {
  TreeNode *t = term();
  while ((token == PLUS) || (token == MINUS)) {
    TreeNode *p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      t->child[1] = term();
    }
  }
  return t;
}

TreeNode *term(void) {
  TreeNode *t = factor();
  while ((token == TIMES) || (token == OVER)) {
    TreeNode *p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      p->child[1] = factor();
    }
  }
  return t;
}

TreeNode *factor(void) {
  TreeNode *t = NULL;
  switch (token) {
  case NUM:
    t = newExpNode(ConstK);
    if ((t != NULL) && (token == NUM))
      t->attr.val = atoi(tokenString);
    match(NUM);
    break;
  case ID:
    t = newExpNode(IdK);
    if ((t != NULL) && (token == ID))
      t->attr.name = copyString(tokenString);
    match(ID);
    break;
  case LPAREN:
    match(LPAREN);
    t = exp();
    match(RPAREN);
    break;
  default:
    syntaxError("token não esperado -> ");
    printToken(token, tokenString);
    token = getToken();
    break;
  }
  return t;
}

/* Função parse retorna a árvore síntatica
 * recentemente construída
 */
TreeNode *parse(void) {
  TreeNode *t;
  token = getToken();
  t = stmt_sequence();
  if (token != ENDFILE)
    syntaxError("Código termina antes do arquivo\n");
  return t;
}
