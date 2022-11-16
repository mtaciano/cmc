/* Implementação da tabela de símbolos */

#include "symtab.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constante `SIZE` é o tamanho da tabela hash
 * seu ideal é ser um número primo
 */
#define SIZE 211

/* Constante `SHIFT` é a potência de dois usada como multiplicador
 * na tabela hash
 */
#define SHIFT 4

/* Função de hash */
static int hash(char *key) {
    int temp = 0;
    int i = 0;

    while (key[i] != '\0') {
        temp = ((temp << SHIFT) + key[i]) % SIZE;
        i++;
    }

    return temp;
}

/* A lista do número de linhas do código fonte
 * em que a variável foi refênciada
 */
typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
} *LineList;

/* O histórico nas bucket lists para cada variável,
 * como nome, memória, escopo, e
 * a lista de número de linhas em que
 * ele aparece no código
 */
typedef struct BucketListRec {
    char *name;
    char *scope;
    char *varfn;
    char *type;
    LineList lines;
    int memloc;
    struct BucketListRec *next;
} *BucketList;

static BucketList hash_table[SIZE];

/* Função `symtab_insert` coloca as linhas,
 * posições de memória e os escopos na tabela de símbolos
 */
void symtab_insert(char *name, char *varfn, char *type, char *scope, int lineno,
                   int memloc) {
    int h = hash(name);
    BucketList l = hash_table[h];

    if (strcmp(varfn, "var") == 0) {
        while ((l != NULL) && ((strcmp(name, l->name) != 0) ||
                               (strcmp(scope, l->scope) != 0))) {
            l = l->next;
        }

        if (l == NULL) {  // se não está na tabela, adicione
            l = malloc(sizeof(*l));
            l->name = name;
            l->scope = scope;
            l->varfn = varfn;
            l->type = type;
            l->lines = malloc(sizeof(*l->lines));
            l->lines->lineno = lineno;
            l->lines->next = NULL;
            l->memloc = memloc;
            l->next = hash_table[h];
            hash_table[h] = l;
        } else {  // encontrou, então adiciona na tabela
            LineList t = l->lines;

            while (t->next != NULL) {
                t = t->next;
            }
            t->next = malloc(sizeof(*t));
            t->next->lineno = lineno;
            t->next->next = NULL;
        }
    } else if (strcmp(varfn, "fun") == 0) {
        while ((l != NULL) && (strcmp(name, l->name) != 0)) {
            l = l->next;
        }

        if (l == NULL) {  // se não está na tabela, adicione
            l = malloc(sizeof(*l));
            l->name = name;
            l->scope = scope;
            l->varfn = varfn;
            l->type = type;
            l->lines = malloc(sizeof(*l->lines));
            l->lines->lineno = lineno;
            l->lines->next = NULL;
            l->memloc = memloc;
            l->next = hash_table[h];
            hash_table[h] = l;
        } else {  // encontrou, então adiciona na tabela
            LineList t = l->lines;
            while (t->next != NULL) {
                t = t->next;
            }
            t->next = malloc(sizeof(*t));
            t->next->lineno = lineno;
            t->next->next = NULL;
        }
    }
}

/* Função `symtab_lookup` retorna a primeira linha de
 * uma variável, e -1 se não encontrar
 */
int symtab_lookup(char *name) {
    int h = hash(name);
    BucketList l = hash_table[h];

    while ((l != NULL) && (strcmp(name, l->name) != 0)) {
        l = l->next;
    }

    if (l == NULL) {
        return -1;
    } else {
        return l->lines->lineno;
    }
}

/* Função `symtab_lookup_scope` retorna a primeira linha de
 * uma variável, e `-1` se não encontrar
 */
int symtab_lookup_scope(char *name, char *scope) {
    for (int i = 0; i < SIZE; i++) {
        BucketList l = hash_table[i];

        while (l != NULL) {
            if (strcmp(name, l->name) == 0 && strcmp(scope, l->scope) == 0) {
                return l->lines->lineno;
            }

            l = l->next;
        }
    }

    return -1;
}

/* Função `symtab_lookup_max_line` retorna o número da linha de
 * uma função, e `-1` se não encontrar
 */
int symtab_lookup_max_line(char *varfn, char *scope) {
    int linhaMax = -1;

    for (int i = 0; i < SIZE; i++) {
        BucketList l = hash_table[i];

        while (l != NULL) {
            if (strcmp(varfn, l->varfn) == 0 && strcmp(scope, l->scope) == 0) {
                if (l->lines->lineno >= linhaMax) {
                    linhaMax = l->lines->lineno;
                }
            }

            l = l->next;
        }
    }

    return linhaMax;
}

/* Função `symtab_print` printa de modo formatado
 * os conteúdos da tabela de símbolos
 * para o arquivo listing
 */
void symtab_print(FILE *listing) {
    fprintf(listing,
            "NOME VARIÁVEL  LOCALIZAÇÃO  ESCOPO  TIPO_ID  TIPO_DADO  LINHAS\n");
    fprintf(listing,
            "-------------  -----------  ------  -------  ---------  ------\n");

    for (int i = 0; i < SIZE; ++i) {
        if (hash_table[i] != NULL) {
            BucketList l = hash_table[i];

            while (l != NULL) {
                LineList t = l->lines;

                fprintf(listing, "%-14s ", l->name);
                fprintf(listing, "%-9d  ", l->memloc);
                fprintf(listing, "%8s", l->scope);
                fprintf(listing, "%7s", l->varfn);
                fprintf(listing, "%10s", l->type);

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
