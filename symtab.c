/****************************************************/
/* File: symtab.c                                   */
/* Implementação da tabela de símbolos              */
/* Miguel Silva Taciano e Gabriel Bianchi e Silva   */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE é o tamanho da tabela hash */
#define SIZE 211

/* SHIFT é a potência de dois usada como multiplicador na tabela hash */
#define SHIFT 4

/* Função de hash */
static int hash(char *key) {
  int temp = 0;
  int i = 0;
  while (key[i] != '\0') {
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* A lista do número de linhas do código fonte
 * em que a variável foi refênciada
 */
typedef struct LineListRec {
  int lineno;
  struct LineListRec *next;
} * LineList;

/* O histórico nas bucket lists para cada variável,
 * como nome, memória, escopo, e
 * a lista de número de linhas em que
 * ele aparece no código
 */
typedef struct BucketListRec {
  char *name;
  char *scope;
  char *varFun;
  char *tipo;
  LineList lines;
  int memloc;
  struct BucketListRec *next;
} * BucketList;

static BucketList hashTable[SIZE];

/* Função st_insert coloca as linhas,
 * posicoes de memoria e os escopos na tabela de simbolos
 */
void st_insert(char *name, char *varFun, char *tipo, char *scope, int lineno,
               int loc) {
  int h = hash(name);
  BucketList l = hashTable[h];
  if (strcmp(varFun, "var") == 0) {
    while ((l != NULL) &&
           ((strcmp(name, l->name) != 0) || (strcmp(scope, l->scope) != 0)))
      l = l->next;
    if (l == NULL) /* se não está na tabela, adicione */
    {
      l = (BucketList)malloc(sizeof(struct BucketListRec));
      l->name = name;
      l->lines = (LineList)malloc(sizeof(struct LineListRec));
      l->scope = scope;
      l->varFun = varFun;
      l->tipo = tipo;
      l->lines->lineno = lineno;
      l->memloc = loc;
      l->lines->next = NULL;
      l->next = hashTable[h];
      hashTable[h] = l;
    } else { /* encontrou, então adiciona na tabela */
      LineList t = l->lines;
      while (t->next != NULL)
        t = t->next;
      t->next = (LineList)malloc(sizeof(struct LineListRec));
      t->next->lineno = lineno;
      t->next->next = NULL;
    }
  } else if (strcmp(varFun, "fun") == 0) {
    while ((l != NULL) && (strcmp(name, l->name) != 0))
      l = l->next;
    if (l == NULL) /* se não está na tabela, adicione */
    {
      l = (BucketList)malloc(sizeof(struct BucketListRec));
      l->name = name;
      l->lines = (LineList)malloc(sizeof(struct LineListRec));
      l->scope = scope;
      l->varFun = varFun;
      l->tipo = tipo;
      l->lines->lineno = lineno;
      l->memloc = loc;
      l->lines->next = NULL;
      l->next = hashTable[h];
      hashTable[h] = l;
    } else { /* encontrou, então adiciona na tabela */
      LineList t = l->lines;
      while (t->next != NULL)
        t = t->next;
      t->next = (LineList)malloc(sizeof(struct LineListRec));
      t->next->lineno = lineno;
      t->next->next = NULL;
    }
  }
} /* st_insert */

/* Função st_lookup retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int st_lookup(char *name) {
  int h = hash(name);
  BucketList l = hashTable[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l == NULL)
    return -1;
  else
    return l->lines->lineno;
}

/* Função st_lookup_scope retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int st_lookup_scope(char *name, char *scope) {
  for (int i = 0; i < SIZE; i++) {
    BucketList l = hashTable[i];
    while (l != NULL) {
      if (strcmp(name, l->name) == 0 && strcmp(scope, l->scope) == 0) {
        return l->lines->lineno;
      }
      l = l->next;
    }
  }
  return -1;
}

/* Função st_lookup_max_linha retorna o número da linha de
 * uma função, e -1 se não encontrar
 */
int st_lookup_max_linha(char *varFun, char *scope) {
  int linhaMax = -1;
  for (int i = 0; i < SIZE; i++) {
    BucketList l = hashTable[i];
    while (l != NULL) {
      if (strcmp(varFun, l->varFun) == 0 && strcmp(scope, l->scope) == 0) {
        if (l->lines->lineno >= linhaMax) {
          linhaMax = l->lines->lineno;
        }
      }
      l = l->next;
    }
  }
  return linhaMax;
}

/* Função printSymTab printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void printSymTab(FILE *listing) {
  int i;
  fprintf(listing,
          "NOME VARIÁVEL  LOCALIZAÇÃO  ESCOPO  TIPO_ID  TIPO_DADO  LINHAS\n");
  fprintf(listing,
          "-------------  -----------  ------  -------  ---------  ------\n");
  for (i = 0; i < SIZE; ++i) {
    if (hashTable[i] != NULL) {
      BucketList l = hashTable[i];
      while (l != NULL) {
        LineList t = l->lines;
        fprintf(listing, "%-14s ", l->name);
        fprintf(listing, "%-9d  ", l->memloc);
        fprintf(listing, "%8s", l->scope);
        fprintf(listing, "%7s", l->varFun);
        fprintf(listing, "%10s", l->tipo);
        while (t != NULL) {
          fprintf(listing, "%7d ", t->lineno);
          t = t->next;
        }
        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
}
