/* Código intermediário */

#include "intermediate.h"
#include "common/globals.h"
#include "common/util.h"

/* Tamanho máximo de uma string */
#define STRING_SIZE 64

/* Quadrupla final */
static Quad quad;

/* Número ainda não usado para um temporário */
static int available_temp_num = 0;

/* Número ainda não usado para um label */
static int available_lab_num = 0;

/* Ordem dos loads */
static CharStack loads;

/* Ordem dos temporários */
static CharStack temps;

/* Ordem dos cálculos */
static CharStack calcs;

/* Ordem dos whiles */
static CharStack whiles;

/* Ordem dos ifs */
static CharStack ifs;

/* O quanto dentro de uma call o programa está */
static int nested_call_level = 0;

/* Se o nó atual está dentro de um bloco de parâmetros */
static int inside_param = FALSE;

/* Função `print_quad` printa a quadrupla inteira */
static void print_quad(Quad q) {
    Quad current = q;

    fprintf(listing, "|%7s, %7s, %7s, %7s|\n\n", "CMD", "ARG1", "ARG2", "ARG3");

    while (current != NULL) {
        fprintf(listing, "(%7s, %7s, %7s, %7s)\n", current->cmd, current->arg1,
                current->arg2, current->arg3);

        current = current->next;
    }
}

/* Função `insert_quad` insere quadruplas na variável quad */
static void insert_quad(char *cmd, char *arg1, char *arg2, char *arg3) {
    Quad new_quad = malloc(sizeof(*new_quad));

    new_quad->cmd = cmd;
    new_quad->arg1 = arg1;
    new_quad->arg2 = arg2;
    new_quad->arg3 = arg3;
    new_quad->next = NULL;

    if (quad == NULL) {
        quad = new_quad;
    } else {
        Quad last_quad = quad;

        while (last_quad->next != NULL) {
            last_quad = last_quad->next;
        }

        last_quad->next = new_quad;
    }
}

/* Função `type_to_string` transforma o tipo em uma string, void ou int */
// TODO: verificar se realmente é necessária uma função para isso
char *type_to_string(TreeNode *t) {
    switch (t->type) {
    case Integer:
        return "INT";
        break;
    case IntegerArray:
        return "INTARR";
        break;
    case Void:
        return "VOID";
        break;
    default: // Não deve acontecer normalmente
        return NULL;
        break;
    }
}

/* Função `read_tree_node` lê o nó da árvore e trata ele de acordo */
/* TODO: tratar todos os casos possíveis */
static void read_tree_node(TreeNode *t) {
    // TODO: melhorar e documentar

    // TODO: remover a maioria do código
    // TODO: transformando read_tree_node em uma função que executa 3 outras
    // TODO: sendo elas uma pré, uma mid e uma pós nó
    // TODO: documentar flags
    int fun_node = FALSE;
    int assign_node = FALSE;
    int const_assign_node = FALSE;
    int inside_call = nested_call_level <= 0 ? FALSE : TRUE;
    int if_node = FALSE;
    int while_node = FALSE;
    int array_node = FALSE;
    int array_assign_node = FALSE;
    int callk_node = FALSE;

    // Verifica o nó atual
    switch (t->node_kind) {
    case StmtK: {
        switch (t->kind.stmt) {
        case IfK:
            if_node = TRUE;
            break;

        case WhileK: {
            char *label = malloc(STRING_SIZE * sizeof(*label));

            snprintf(label, STRING_SIZE, ".LB%d", available_lab_num);
            insert_quad("LAB", label, "--", "--");
            cs_push(whiles, label);

            while_node = TRUE;
            available_lab_num++;
        } break;

        case AssignK:
            assign_node = TRUE;
            if (t->child[0]->node_kind == ExpK &&
                t->child[0]->kind.exp == ArrIdK) {
                array_assign_node = TRUE;
            }

            if (t->child[1]->node_kind == ExpK &&
                t->child[1]->kind.exp == ConstK) {
                const_assign_node = TRUE;
            }
            break;

        case CompoundK:
            /* todo */
            break;

        case ReturnK:
            /* todo */
            break;
        }
    } break;
    case ExpK: {
        switch (t->kind.exp) {
        case OpK: {
            switch (t->attr.op) {
            case LT:
                /* todo */
                break;

            case LE:
                /* todo */
                break;

            case GT:
                /* todo */
                break;

            case GE:
                /* todo */
                break;

            case EQ:
                /* todo */
                break;

            case NE:
                /* todo */
                break;

            case PLUS:
                /* todo */
                break;

            case MINUS:
                /* todo */
                break;

            case TIMES:
                /* todo */
                break;

            case OVER:
                /* todo */
                break;
            }
        } break;

        case ConstK: {
            char *temp = malloc(STRING_SIZE * sizeof(*temp));
            char *c = malloc(STRING_SIZE * sizeof(*c));

            snprintf(c, STRING_SIZE, "%d", t->attr.val);
            snprintf(temp, STRING_SIZE, "$t%d", available_temp_num);
            cs_push(temps, temp);
            insert_quad("LOAD", temp, c, "--");

            available_temp_num++;
        } break;

        case IdK: {
            if (inside_call || !fun_node) {
                char *temp = malloc(STRING_SIZE * sizeof(*temp));

                snprintf(temp, STRING_SIZE, "$t%d", available_temp_num);
                cs_push(temps, temp);
                insert_quad("LOAD", temp, t->attr.name, "--");

                available_temp_num++;
            }
        } break;

        case TypeK:
            /* todo */
            break;

        case ArrIdK:
            array_node = TRUE;
            break;

        case CallK:
            callk_node = TRUE;
            inside_call = TRUE;
            nested_call_level++;
            break;

        case CalcK:
            break;
        }
    } break;

    case DeclK: {
        switch (t->kind.decl) {
        case VarK: {
            char *name = t->attr.name;

            insert_quad("ALLOC", name, t->scope, "--");
        } break;

        case FunK: {
            char *name = t->attr.name;
            char *temp = type_to_string(t->child[0]);

            insert_quad("FUN", temp, name, "--");

            fun_node = TRUE;
        } break;

        case ArrVarK: {
            char *name = t->attr.arr.name;
            char *size = malloc(STRING_SIZE * sizeof(*size));

            snprintf(size, STRING_SIZE, "%d", t->attr.arr.size);
            insert_quad("ARRLOC", name, t->scope, size);
        } break;

        case ArrParamK:
            /* todo */
            break;

        case ParamK: {
            if (t->child[0] != NULL) {
                char *temp = type_to_string(t->child[0]);
                char *name = t->attr.name;
                char *scope = t->scope;

                insert_quad("ARG", temp, name, scope);
            }
        } break;
        }
        break;
    }
    }

    // Verifica os 3 filhos
    for (int i = 0; i < 3; i++) {
        if (t->child[i] != NULL) {
            if (t->child[i]->node_kind == ExpK &&
                t->child[i]->kind.exp == TypeK) {
                continue; // É do tipo `Type`, não há necessidade de continuar
            }
            if (assign_node && const_assign_node && i == 1) {
                continue; // É do tipo `Id`, não há necessidade de continuar
            }

            if (if_node && i == 1) { // Antes de entrar no `if`
                char *temp = cs_pop(temps);
                char *label = malloc(STRING_SIZE * sizeof(*label));

                snprintf(label, STRING_SIZE, ".LB%d", available_lab_num);
                insert_quad("IFF", temp, label, "--");

                cs_push(ifs, label);

                available_lab_num++;
            }

            if (while_node && i == 1) { // Antes de entrar no `while`
                char *temp = cs_pop(temps);
                char *label = malloc(STRING_SIZE * sizeof(*label));

                snprintf(label, STRING_SIZE, ".LB%d", available_lab_num);
                insert_quad("IFF", temp, label, "--");

                cs_push(whiles, label);

                available_lab_num++;
            }

            // Chamada dos filhos do nó
            if (callk_node) { // CallK que chama
                inside_param = TRUE;
                read_tree_node(t->child[i]);
            } else if (inside_param) { // Filhos de CallK chamam
                inside_param = FALSE;
                read_tree_node(t->child[i]);
                inside_param = TRUE;
            } else { // Outros nós chamam
                read_tree_node(t->child[i]);
            }

            if (if_node && i == 1) { // Saindo do `if`, entrando no `else`
                // Como há uma comparação se é `NULL` no começo do `for`
                // então não tem como realizar estes passos
                // durante a entrada no `else`
                if (t->child[2] != NULL) { // Tem `else`, fazer pop() -> push()
                    char *l1 = cs_pop(ifs);
                    char *l2 = malloc(STRING_SIZE * sizeof(*l2));

                    snprintf(l2, STRING_SIZE, ".LB%d", available_lab_num);
                    insert_quad("GOTO", l2, "--", "--");
                    insert_quad("LAB", l1, "--", "--");

                    cs_push(ifs, l2);

                    available_lab_num++;
                } else { // Não tem else, apenas fazer pop()
                    insert_quad("LAB", cs_pop(ifs), "--", "--");
                }
            } else if (if_node && i == 2) { // Saindo do `else`
                // Como acontece uma comparação com `NULL` no começo do `for`
                // esse caso só acontece se existe um `else`
                // então não é necessário verificar se é NULL
                insert_quad("LAB", cs_pop(ifs), "--", "--");
            }

            if (while_node && i == 1) { // Saindo do `while`
                char *l2 = cs_pop(whiles);
                char *l1 = cs_pop(whiles);

                insert_quad("GOTO", l1, "--", "--");
                insert_quad("LAB", l2, "--", "--");
            }

            if (array_node) {
                char *name = malloc(STRING_SIZE * sizeof(*name));
                char *temp = malloc(STRING_SIZE * sizeof(*temp));
                char *offset = malloc(STRING_SIZE * sizeof(*offset));

                snprintf(temp, STRING_SIZE, "$t%d", available_temp_num);
                strcpy(offset, temps->items[temps->last]);
                strcpy(name, t->attr.name);
                insert_quad("ARRLOAD", temp, name, offset);
                cs_push(temps, temp);

                available_temp_num++;
            }
        }
    }

    if (fun_node) {
        insert_quad("END", t->attr.name, "--", "--");
    }

    if (assign_node) {
        if (const_assign_node) {
            char *temp = cs_pop(temps);
            char *val = malloc(STRING_SIZE * sizeof(*val));
            char *name = t->child[0]->attr.name;

            snprintf(val, STRING_SIZE, "%d", t->child[1]->attr.val);
            insert_quad("ASSIGN", temp, val, "--");

            if (array_assign_node) {
                insert_quad("ARRSTR", name, temp, cs_pop(temps));
            } else {
                insert_quad("STORE", name, temp, "--");
            }
        } else {
            char *t1 = cs_pop(temps);
            char *t2 = cs_pop(temps);
            char *name = t->child[0]->attr.name;

            insert_quad("ASSIGN", t2, t1, "--");

            if (array_assign_node) {
                insert_quad("ARRSTR", name, t2, cs_pop(temps));
            } else {
                insert_quad("STORE", name, t2, "--");
            }
        }
    }

    if (inside_param && !callk_node && t->sibling != NULL) {
        insert_quad("PARAM", cs_pop(temps), "--", "--");
    }

    // Verifica o irmão
    if (t->sibling != NULL) {
        read_tree_node(t->sibling);
    }

    // Adiciona parâmetro no stack
    if (t->node_kind == DeclK && t->kind.decl == ParamK &&
        t->child[0] != NULL) {
        cs_push(loads, t->attr.name);
    }

    switch (t->node_kind) {
    case ExpK: {
        switch (t->kind.exp) {
        case OpK: {
            switch (t->attr.op) {
            case LT:
                cs_push(calcs, "LESS");
                break;

            case LE:
                cs_push(calcs, "LEQ");
                break;

            case GT:
                cs_push(calcs, "GREAT");
                break;

            case GE:
                cs_push(calcs, "GRTEQ");
                break;

            case EQ:
                cs_push(calcs, "EQUAL");
                break;

            case NE:
                cs_push(calcs, "NOTEQ");
                break;

            case PLUS:
                cs_push(calcs, "ADD");
                break;

            case MINUS:
                cs_push(calcs, "SUB");
                break;

            case TIMES:
                cs_push(calcs, "MULT");
                break;

            case OVER:
                cs_push(calcs, "DIV");
                break;
            }
        } break;

        case CalcK: {
            char *t1 = cs_pop(temps);
            char *t2 = cs_pop(temps);
            char *t3 = malloc(STRING_SIZE * sizeof(*t3));
            char *cmd = cs_pop(calcs);

            snprintf(t3, STRING_SIZE, "$t%d", available_temp_num);
            cs_push(temps, t3);
            insert_quad(cmd, t3, t2, t1);

            available_temp_num++;
        } break;

        case ArrIdK:
            array_node = TRUE;
            break;

        case CallK: {
            if (inside_param &&
                !(t->node_kind == ExpK && t->kind.exp == CalcK)) {
                if (t->node_kind == ExpK && t->kind.exp == CallK) {
                    if (nested_call_level <= 0) {
                        inside_param = FALSE;
                    }
                }

                insert_quad("PARAM", cs_pop(temps), "--", "--");
            }

            callk_node = FALSE;
            nested_call_level--;

            if (nested_call_level <= 0) { // Ultima call de todas
                nested_call_level = 0;
                inside_param = FALSE;
                inside_call = FALSE;
            }

            // Calculando quantos parâmetros a função tem
            int param_num = 0;
            TreeNode *curr_node = t->child[0];

            while (curr_node != NULL) {
                curr_node = curr_node->sibling;
                param_num++;
            }

            char *temp = malloc(STRING_SIZE * sizeof(*temp));
            char *p_num = malloc(STRING_SIZE * sizeof(*p_num));

            snprintf(temp, STRING_SIZE, "$t%d", available_temp_num);
            cs_push(temps, temp);
            snprintf(p_num, STRING_SIZE, "%d", param_num);
            insert_quad("CALL", temp, t->attr.name, p_num);

            available_temp_num++;
        } break;

        case IdK:
            /* code */
            break;

        default:
            break;
        }
    } break;

    case StmtK: {
        switch (t->kind.stmt) {
        case ReturnK:
            insert_quad("RET", cs_pop(temps), "--", "--");
            break;

        default:
            break;
        }
    } break;

    case DeclK: {
        /* code */
        break;
    }
    }
}

/* Função `make_intermediate` é responsável por gerar o código intermediário */
Quad make_intermediate(TreeNode *t) {
    // Inicializando as pilhas
    loads = cs_init();
    temps = cs_init();
    calcs = cs_init();
    whiles = cs_init();
    ifs = cs_init();

    read_tree_node(t);
    insert_quad("HALT", "--", "--", "--");

    if (g_trace_code) {
        print_quad(quad);
    }

    // Removendo todas as pilhas da memória
    cs_drop(loads);
    cs_drop(temps);
    cs_drop(calcs);
    cs_drop(whiles);
    cs_drop(ifs);

    return quad;
}
