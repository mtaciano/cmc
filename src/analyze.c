/* Análise Semântica da linguagem C- */

#include "analyze.h"

#include <stdbool.h>

#include "common/globals.h"
#include "common/util.h"
#include "symtab.h"

/* Contador da posição na memória */
static int memloc = 0;

/* Se a funcão precisa de um `return`,
 * usado para verificar se uma função que precisa de um return não tem um
 */
static int fn_has_return = false;

/* Escopos das variáveis e funções */
static CharStack scopes;

/* Função `symbol_error` imprime um erro durante a tabela de símbolos,
 * tal erro também é um erro semântico
 */
static void symbol_error(TreeNode *t, char *message) {
    fprintf(listing, "ERRO SEMÂNTICO: %s LINHA: %d\n", message, t->lineno);

    g_error = true;
    exit(-1);
}

/* Função `verify_main` verifica se a `main` é a ultima declaração de funcão */
static void verify_main(void) {
    int max_lineno = symtab_lookup_max_line("fun", "global");
    int max_var_lineno = symtab_lookup_max_line("var", "global");
    int main_lineno = symtab_lookup("main");

    if (max_lineno > main_lineno || max_var_lineno > main_lineno) {
        int max = max_lineno > max_var_lineno ? max_lineno : max_var_lineno;
        fprintf(listing,
                "ERRO SEMÂNTICO: declaração depois do main LINHA: %d\n", max);

        g_error = true;
        exit(-1);
    }
}

/* Função `find_return` verifica o nó atual é um `return` */
static void find_return(TreeNode *t) {
    if (t != NULL) {
        if (t->node_kind == StmtK && t->kind.stmt == ReturnK) {
            fn_has_return = true;
        }
    }
}

/* Função `give_scope` recursivamente cria escopo para os nós */
static void give_scope(TreeNode *t) {
    if (t != NULL) {
        char *scope = scopes->items[scopes->last];

        t->scope = malloc(strlen(scope) * sizeof(scope));
        strcpy(t->scope, scope);

        if (t->node_kind == DeclK && t->kind.decl == FunK) {
            cs_push(scopes, t->attr.name);
        }
    }
}

/* Função `pop_stack` remove o último elemento da pilha */
static void pop_stack(TreeNode *t) {
    if (t != NULL) {
        if (t->node_kind == DeclK && t->kind.decl == FunK) {
            char *item = cs_pop(scopes);
            free(item);
        }
    }
}

/* Função `traverse` é uma função genérica que aplica
 * a função `pre_fn` quando está descendo,
 * e aplica `post_fn` quando está voltando
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

/* Funcão `null_fn` é uma função que não faz nada.
 * Ela é usada para poder fazer traversias de apenas pré ou pós ordem
 * na árvore sintática
 */
static void null_fn(TreeNode *t) {
    (void)t;  // Suprime os avisos de variável não usada

    return;
}

/* Função `insert_node` insere o nó `t` na tabela de símbolos */
static void insert_node(TreeNode *t) {
    char *varfn;
    char *type;

    switch (t->node_kind) {
        case StmtK:
            switch (t->kind.stmt) {
                case AssignK:
                    if (symtab_lookup_scope(t->child[0]->attr.name, t->scope) ==
                            -1 &&
                        symtab_lookup_scope(t->child[0]->attr.name, "global") ==
                            -1) {
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
                    varfn = "var";
                    type = "int";

                    if (symtab_lookup(t->attr.name) == -1) {
                        // Não está na tabela, nova definição
                        symtab_insert(t->attr.name, varfn, type, t->scope,
                                      t->lineno, memloc++);
                    } else {
                        // Já na tabela
                        symtab_insert(t->attr.name, varfn, type, t->scope,
                                      t->lineno, 0);
                    }
                    break;

                case CallK: {
                    if (t->kind.exp == CallK) {
                        varfn = "fun";
                        if (t->child[0] != NULL && t->child[0]->type == Void) {
                            type = "void";
                        } else if (t->child[0] != NULL &&
                                   t->child[0]->type == Integer) {
                            type = "int";
                        } else {
                            type = "error";
                        }
                    }

                    if (symtab_lookup(t->attr.name) == -1) {
                        // Não está na tabela, nova definição
                        symtab_insert(t->attr.name, varfn, type, t->scope,
                                      t->lineno, memloc++);
                    } else {
                        // Já na tabela
                        symtab_insert(t->attr.name, varfn, type, t->scope,
                                      t->lineno, 0);
                    }
                } break;

                default:
                    break;
            }
            break;

        case DeclK:
            switch (t->kind.decl) {
                case FunK:
                    if (symtab_lookup(t->attr.name) != -1) {
                        symbol_error(t, "Redefinição de função");
                        break;
                    }

                    varfn = "fun";
                    if (t->child[0] != NULL && t->child[0]->type == Void) {
                        type = "void";
                    } else if (t->child[0] != NULL &&
                               t->child[0]->type == Integer) {
                        type = "int";
                    } else {
                        type = "error";
                    }

                    if (symtab_lookup(t->attr.name) == -1) {
                        // Não está na tabela, nova definição
                        symtab_insert(t->attr.name, varfn, type, t->scope,
                                      t->lineno, memloc++);
                    }
                    break;

                case VarK:
                    varfn = "var";
                    type = "int";

                    // Verificar se a variável já existe
                    if (symtab_lookup_scope(t->attr.name, t->scope) != -1) {
                        symbol_error(t, "Redefinição de variável");
                        break;
                    }
                    // Tipo não deve ser VOID
                    if (t->child[0]->type == Void) {
                        symbol_error(t, "Variável não pode ser do tipo void");
                        break;
                    }

                    symtab_insert(t->attr.name, varfn, type, t->scope,
                                  t->lineno, memloc++);
                    break;

                case ArrVarK:
                    varfn = "var";
                    type = "int";
                    // Tipo não deve ser VOID
                    if (t->child[0]->type == Void) {
                        symbol_error(
                            t, "Variável do tipo array não pode ser void");
                        break;
                    }

                    // Verificar se a variável  array já foi declarada
                    if (symtab_lookup_scope(t->attr.arr.name, t->scope) != -1) {
                        symbol_error(t, "Variável do tipo array já declarada");
                        break;
                    }

                    symtab_insert(t->attr.arr.name, varfn, type, t->scope,
                                  t->lineno, memloc++);
                    break;

                case ArrParamK:
                    if (t->attr.name == NULL) {
                        break;
                    }

                    varfn = "var";
                    type = "int";

                    // Tipo não deve ser VOID
                    if (t->child[0]->type == Void) {
                        symbol_error(
                            t, "Parâmetro do tipo array não pode ser void");
                        break;
                    }

                    // Verifica se o array já foi declarado
                    if (symtab_lookup_scope(t->attr.name, t->scope) != -1) {
                        symbol_error(t, "Parâmetro do tipo array já declarado");
                        break;
                    }

                    symtab_insert(t->attr.name, varfn, type, t->scope,
                                  t->lineno, memloc++);
                    break;

                case ParamK:
                    if (t->attr.name != NULL) {
                        // Verifica se o parâmetro existe ou é void
                        if (t->type != Void) {
                            if (t->child[0]->type == Void) {
                                symbol_error(
                                    t, "Parâmetro não pode ser do tipo void");
                                break;
                            }
                            if (symtab_lookup_scope(t->attr.name, t->scope) !=
                                -1) {
                                symbol_error(t, "Redefinição de parâmetro");
                                break;
                            }

                            varfn = "var";
                            type = "int";
                            symtab_insert(t->attr.name, varfn, type, t->scope,
                                          t->lineno, memloc++);
                        }
                        break;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    varfn = NULL;
    type = NULL;
}

/* Função `build_symtab` constrói a tabela de símbolos
 * através de uma passada em pré ordem da árvore sintática
 */
void build_symtab(TreeNode *syntax_tree) {
    // Coloca `input()` e `output()` na tabela de símbolos
    // pois ambos são implícitamente declarados
    symtab_insert("input", "fun", "int", "global", 0, memloc++);
    symtab_insert("output", "fun", "void", "global", 0, memloc++);

    scopes = cs_init();
    // O maior escopo que uma variável pode ter é global
    cs_push(scopes, "global");

    traverse(syntax_tree, give_scope, pop_stack);
    traverse(syntax_tree, insert_node, null_fn);

    verify_main();

    cs_drop(scopes);

    if (!g_error && g_trace_analyze) {
        fprintf(listing, "\nTabela de símbolos:\n\n");
        symtab_print(listing);
    }
}

/* Função `check_node` realiza verificação do nó atual */
static void check_node(TreeNode *t) {
    switch (t->node_kind) {
        case StmtK:
            switch (t->kind.stmt) {
                case AssignK: {
                    if (symtab_lookup_scope(t->child[0]->attr.name, t->scope) ==
                            -1 &&
                        symtab_lookup_scope(t->child[0]->attr.name, "global") ==
                            -1) {
                        symbol_error(t->child[0], "Váriavel não declarada");
                    }
                } break;

                default:
                    break;
            }
            break;

        case ExpK:
            switch (t->kind.exp) {
                case IdK:
                case ArrIdK:
                case CallK: {
                    if (symtab_lookup(t->attr.name) == -1) {
                        symbol_error(t, "Símbolo não declarado");
                    }
                } break;

                default:
                    break;
            }
            break;

        case DeclK:
            switch (t->kind.decl) {
                case FunK:
                    // Verifica se a função tem um return se ela precisa de um
                    if (t->child[2] != NULL && t->child[2]->child[1] != NULL) {
                        traverse(t->child[2], find_return, null_fn);
                        if (t->child[0] != NULL && t->child[0]->type == Void) {
                            if (fn_has_return) {
                                symbol_error(
                                    t, "Nenhum valor de retorno esperado");
                            }
                        } else if (t->child[0] != NULL &&
                                   t->child[0]->type == Integer) {
                            if (!fn_has_return) {
                                symbol_error(t, "Valor de retorno esperado");
                            }
                        }
                        fn_has_return = false;
                    }
                    break;

                default:
                    break;
            }

        default:
            break;
    }
}

/* Função `type_check` faz _type-checking_
 * através uma passada de pós ordem
 */
void type_check(TreeNode *syntax_tree) {
    traverse(syntax_tree, null_fn, check_node);
}
