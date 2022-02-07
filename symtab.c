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
  LineList lines;
  int memloc;
  struct BucketListRec *next;
} * BucketList;

static BucketList hashTable[SIZE];

/* Função st_insert coloca as linhas,
 * posicoes de memoria e os escopos na tabela de simbolos
 */
void st_insert(char *name, char *scope, int lineno, int loc) {
  int h = hash(name);
  BucketList l = hashTable[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l == NULL) /* se não está na tabela, adicione */
  {
    l = (BucketList)malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList)malloc(sizeof(struct LineListRec));
    l->scope = scope;
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
} /* st_insert */

/* Função st_lookup retorna a posição na memória de
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
    return l->memloc;
}

/* Função printSymTab printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void printSymTab(FILE *listing) {
  int i;
  fprintf(listing, "NOME VARIÁVEL  LOCALIZAÇÃO  ESCOPO  LINHAS\n");
  fprintf(listing, "-------------  -----------  ------  ------\n");
  for (i = 0; i < SIZE; ++i) {
    if (hashTable[i] != NULL) {
      BucketList l = hashTable[i];
      while (l != NULL) {
        LineList t = l->lines;
        fprintf(listing, "%-14s ", l->name);
        fprintf(listing, "%-9d  ", l->memloc);
        fprintf(listing, "%8s", l->scope);
        while (t != NULL) {
          fprintf(listing, "%4d ", t->lineno);
          t = t->next;
        }
        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
}
