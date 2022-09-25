/* Analise Semântica da linguagem C- */

#include "analyze.h"
#include "common/globals.h"
#include "common/util.h"
#include "symtab.h"

#define STACK_SIZE 256
#define STRING_SIZE 256

/* Contador da posição na memória */
static int location = 0;

/* Se a funcão precisa de um return,
 * usado para verificar se uma função que precisa de um return não tem um
 */
static int fn_has_return = FALSE;

/* Componentes da pilha */
typedef struct {
    int max_size;
    int last;
    char **items;
} * CharStack;

/* Função cs_init inicia a pilha com um tamanho máximo de `size` */
CharStack cs_init() {
    int size = 256;

    CharStack new = malloc(sizeof(*new));
    new->max_size = size;
    new->last = -1;
    new->items = malloc(size * sizeof(**new->items));

    return new;
}

/* Função cs_push coloca a string na pilha */
void cs_push(CharStack stack, char *item) {
    if (stack->last >= stack->max_size) { // Redimencionar
        stack->max_size *= 2;
        stack->items =
            realloc(stack->items, stack->max_size * sizeof(**stack->items));
    }

    stack->last += 1;
    stack->items[stack->last] = malloc(sizeof(item));
    strcpy(stack->items[stack->last], item);
}

/* Função cs_pop remove a string da pilha, retornando seu valor */
char *cs_pop(CharStack stack) {
    char *item = stack->items[stack->last];

    stack->items[stack->last] = NULL;
    stack->last -= 1;

    return item;
}

/* Função cs_drop remove a memória usada pela pilha */
void cs_drop(CharStack stack) {
    for (int i = 0; i <= stack->last; i++) {
        free(stack->items[i]);
    }

    free(stack->items);
    free(stack);
}

static CharStack scopes;

/* Função symbol_error imprime um erro durante a tabela de símbolos,
 * tal erro também é um erro semântico
 */
static void symbol_error(TreeNode *t, char *message) {
    fprintf(listing, "ERRO SEMÂNTICO: %s LINHA: %d\n", message, t->lineno);

    Error = TRUE;
    exit(-1);
}

/* Função verify_main verifica se a `main` é a ultima declaração de funcão */
static void verify_main() {
    int max_lineno = st_lookup_max_line("fun", "global");
    int max_var_lineno = st_lookup_max_line("var", "global");
    int main_lineno = st_lookup("main");

    if (max_lineno > main_lineno || max_var_lineno > main_lineno) {
        int max = max_lineno > max_var_lineno ? max_lineno : max_var_lineno;
        fprintf(listing,
                "ERRO SEMÂNTICO: declaração depois do main LINHA: %d\n", max);

        Error = TRUE;
        exit(-1);
    }
}

/* Função find_return verifica o nó atual é um `return` */
static void find_return(TreeNode *t) {
    if (t != NULL) {
        if (t->nodekind == StmtK && t->kind.stmt == ReturnK) {
            fn_has_return = TRUE;
        }
    }
}

/* Função give_scope recursivamente cria escopo para os nós */
static void give_scope(TreeNode *t) {
    if (t != NULL) {
        t->scope = malloc(sizeof(scopes->items[scopes->last]));

        if (t->nodekind == DeclK && t->kind.decl == FunK) {
            strcpy(t->scope, scopes->items[scopes->last]);
            cs_push(scopes, t->attr.name);
        } else {
            strcpy(t->scope, scopes->items[scopes->last]);
        }
    }
}

/* Função pop_stack remove o ultimo elemento da pilha */
static void pop_stack(TreeNode *t) {
    if (t != NULL) {
        if (t->nodekind == DeclK && t->kind.decl == FunK) {
            char *item = cs_pop(scopes);
            free(item);
        }
    }
}

static void insert_input(void) {
    // TreeNode *fun = new_DeclNode(FunK);
    // fun->type = Integer;

    // TreeNode *type = new_ExpNode(TypeK);
    // type->attr.type = INT;

    // TreeNode *compound_stmt = new_StmtNode(CompoundK);
    // compound_stmt->child[0] = NULL;
    // compound_stmt->child[1] = NULL;

    // fun->lineno = 0;
    // fun->attr.name = "input";
    // fun->child[0] = type;
    // fun->child[1] = NULL;
    // fun->child[2] = compound_stmt;

    /* Coloca o input na tabela de símbolos */
    st_insert("input", "fun", "int", "global", 0, location++);
}

static void insert_output(void) {
    // TreeNode *fun = new_DeclNode(FunK);
    // fun->type = Void;

    // TreeNode *type = new_DeclNode(FunK);
    // type->attr.type = VOID;

    // TreeNode *params = new_DeclNode(ParamK);
    // params->attr.name = "x";
    // params->child[0] = new_ExpNode(TypeK);
    // params->child[0]->attr.type = INT;

    // TreeNode *compound_stmt = new_StmtNode(CompoundK);
    // compound_stmt->child[0] = NULL;
    // compound_stmt->child[1] = NULL;

    // fun->lineno = 0;
    // fun->attr.name = "output";
    // fun->child[0] = type;
    // fun->child[1] = params;
    // fun->child[2] = compound_stmt;

    /* Coloca o output na tabela de símbolos */
    st_insert("output", "fun", "void", "global", 0, location++);
}

/* Função traverse é uma função genérica que aplica
 * a função pre_fn quando está descendo,
 * e aplica post_fn quando está voltando
 */
static void traverse(TreeNode *t, void (*pre_fn)(TreeNode *),
                     void (*post_fn)(TreeNode *)) {
    if (t != NULL) {
        pre_fn(t);

        for (int i = 0; i < MAXCHILDREN; i++) {
            traverse(t->child[i], pre_fn, post_fn);
        }

        post_fn(t);

        traverse(t->sibling, pre_fn, post_fn);
    }
}

/* Funcão null_fn é uma função que não faz nada.
 * Ela é usada para poder fazer traversias de apenas pré ou pós ordem
 * na árvore sintática
 */
static void null_fn(TreeNode *t) {
    (void)t; // Suprime os avisos de variável não usada

    return;
}

/* Função insert_node insere o nó `t` na tabela de símbolos */
static void insert_node(TreeNode *t) {
    char *var_or_fun;
    char *type;

    switch (t->nodekind) {
    case StmtK:
        switch (t->kind.stmt) {
        case AssignK:
            if (st_lookup_scope(t->child[0]->attr.name, t->scope) == -1 &&
                st_lookup_scope(t->child[0]->attr.name, "global") == -1) {
                symbol_error(t, "Váriável não declarada");
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
            var_or_fun = "var";
            type = "int";
        case CallK: {
            if (t->kind.exp == CallK) {
                var_or_fun = "fun";
                if (t->child[0] != NULL && t->child[0]->type == Void) {
                    type = "void";
                } else if (t->child[0] != NULL &&
                           t->child[0]->type == Integer) {
                    type = "int";
                }
            }

            if (st_lookup(t->attr.name) == -1) {
                // Não está na tabela, nova definição
                st_insert(t->attr.name, var_or_fun, type, t->scope, t->lineno,
                          location++);
            } else
                // Já na tabela
                st_insert(t->attr.name, var_or_fun, type, t->scope, t->lineno,
                          0);
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
                symbol_error(t, "Redefinição de função");
                break;
            }

            var_or_fun = "fun";
            if (t->child[0] != NULL && t->child[0]->type == Void) {
                type = "void";
            } else if (t->child[0] != NULL && t->child[0]->type == Integer) {
                type = "int";
            }

            if (st_lookup(t->attr.name) == -1) {
                // Não está na tabela, nova definição
                st_insert(t->attr.name, var_or_fun, type, t->scope, t->lineno,
                          location++);
            }
            break;
        case VarK:
            var_or_fun = "var";
            type = "int";

            // Verificar se a variável já existe
            if (st_lookup_scope(t->attr.name, t->scope) != -1) {
                symbol_error(t, "Redefinição de variável");
                break;
            }
            // Tipo não deve ser VOID
            if (t->child[0]->type == Void) {
                symbol_error(t, "Variável não pode ser do tipo void");
                break;
            }

            st_insert(t->attr.name, var_or_fun, type, t->scope, t->lineno,
                      location++);
            break;
        case ArrVarK:
            var_or_fun = "var";
            type = "int";
            // Tipo não deve ser VOID
            if (t->child[0]->type == Void) {
                symbol_error(t, "Variável do tipo array não pode ser void");
                break;
            }

            // Verificar se a variável  array já foi declarada
            if (st_lookup_scope(t->attr.arr.name, t->scope) != -1) {
                symbol_error(t, "Variável do tipo array já declarada");
                break;
            }

            st_insert(t->attr.arr.name, var_or_fun, type, t->scope, t->lineno,
                      location++);
            break;
        case ArrParamK:
            if (t->attr.name == NULL) {
                break;
            }

            var_or_fun = "var";
            type = "int";

            // Tipo não deve ser VOID
            if (t->child[0]->type == Void) {
                symbol_error(t, "Parâmetro do tipo array não pode ser void");
                break;
            }

            // Verifica se o array já foi declarado
            if (st_lookup_scope(t->attr.name, t->scope) != -1) {
                symbol_error(t, "Parâmetro do tipo array já declarado");
                break;
            }

            st_insert(t->attr.name, var_or_fun, type, t->scope, t->lineno,
                      location++);
            break;
        case ParamK:
            if (t->attr.name != NULL) {
                // Verifica se o parâmetro existe ou é void
                if (t->type != Void) {
                    if (t->child[0]->type == Void) {
                        symbol_error(t, "Parâmetro não pode ser do tipo void");
                        break;
                    }
                    if (st_lookup_scope(t->attr.name, t->scope) != -1) {
                        symbol_error(t, "Redefinição de parâmetro");
                        break;
                    }

                    var_or_fun = "var";
                    type = "int";
                    st_insert(t->attr.name, var_or_fun, type, t->scope,
                              t->lineno, location++);
                }
                break;
            }
            break;
        default:
            break;
        }
    default:
        break;
    }

    var_or_fun = "\0";
    type = "\0";
}

/* Função build_symbol_table constrói a tabela de símbolos
 * através de uma passada em pré ordem da árvore sintática
 */
void build_symbol_table(TreeNode *syntax_tree) {
    insert_input();
    insert_output();

    scopes = cs_init();
    // O maior escopo que uma variável pode ter é global
    cs_push(scopes, "global");

    traverse(syntax_tree, give_scope, pop_stack);
    traverse(syntax_tree, insert_node, null_fn);

    verify_main();

    cs_drop(scopes);

    if (!Error && TraceAnalyze) {
        fprintf(listing, "\nTabela de símbolos:\n\n");
        print_symbol_table(listing);
    }
}

/* Função check_node realiza verificação do nó atual */
static void check_node(TreeNode *t) {
    switch (t->nodekind) {
    case StmtK:
        switch (t->kind.stmt) {
        case AssignK: {
            if (st_lookup_scope(t->child[0]->attr.name, t->scope) == -1 &&
                st_lookup_scope(t->child[0]->attr.name, "global") == -1) {
                symbol_error(t->child[0], "Váriavel não declarada");
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
                symbol_error(t, "Símbolo não declarado");
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
                traverse(t->child[2], find_return, null_fn);
                if (t->child[0] != NULL && t->child[0]->type == Void) {
                    if (fn_has_return) {
                        symbol_error(t, "Nenhum valor de retorno esperado");
                    }
                } else if (t->child[0] != NULL &&
                           t->child[0]->type == Integer) {
                    if (!fn_has_return) {
                        symbol_error(t, "Valor de retorno esperado");
                    }
                }
                fn_has_return = FALSE;
            }
            break;
        default:
            break;
        }
    default:
        break;
    }
}

/* Função type_check faz _type-checking_
 * através uma passada de pós ordem
 */
void type_check(TreeNode *syntax_tree) {
    traverse(syntax_tree, null_fn, check_node);
}
