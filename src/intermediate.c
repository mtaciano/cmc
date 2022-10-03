/* Código intermediário */
// TODO: desistir, impossível melhorar isso

#include "intermediate.h"
#include "common/globals.h"
#include "common/util.h"

/* Tamanho máximo de uma string */
#define STRING_SIZE 64

/* Quadrupla final */
static Quad quad;

/* Número ainda não usado para um temporário */
static int temp_num = 0;

/* Número ainda não usado para um label */
static int lab_num = 0;

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
static int is_inside_param = FALSE;

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
    int is_fun = FALSE;
    int is_asn = FALSE;
    int is_const_asn = FALSE;
    int is_call;
    int is_iff = FALSE;
    int is_while = FALSE;
    int is_arr = FALSE;
    int is_arr_asn = FALSE;
    int is_curr_callk = FALSE;

    if (nested_call_level <= 0) {
        is_call = FALSE;
    } else {
        is_call = TRUE;
    }

    // Verifica o nó atual
    switch (t->node_kind) {
    case StmtK: {
        switch (t->kind.stmt) {
        case IfK:
            is_iff = TRUE;
            break;

        case WhileK: {
            char *label = malloc(STRING_SIZE * sizeof(*label));
            snprintf(label, STRING_SIZE, ".LB%d", lab_num);
            insert_quad("LAB", label, "--", "--");

            is_while = TRUE;
            lab_num++;
            cs_push(whiles, label);
        } break;

        case AssignK:
            is_asn = TRUE;
            if (t->child[0]->node_kind == ExpK &&
                t->child[0]->kind.exp == ArrIdK) {
                is_arr_asn = TRUE;
            }

            if (t->child[1]->node_kind == ExpK &&
                t->child[1]->kind.exp == ConstK) {
                is_const_asn = TRUE;
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
            snprintf(temp, STRING_SIZE, "$t%d", temp_num);
            cs_push(temps, temp);
            // push(stack_temp, temp, &stack_temp_last);
            insert_quad("LOAD", temp, c, "--");
            temp_num++;
        } break;
        case IdK: {
            if (is_call || !is_fun) {
                char *temp = malloc(STRING_SIZE * sizeof(*temp));
                snprintf(temp, STRING_SIZE, "$t%d", temp_num);
                cs_push(temps, temp);
                temp_num++;
                insert_quad("LOAD", temp, t->attr.name, "--");
            }
        } break;
        case TypeK:
            /* todo */
            break;
        case ArrIdK:
            is_arr = TRUE;
            break;
        case CallK:
            is_curr_callk = TRUE;
            is_call = TRUE;
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
            is_fun = TRUE;
            insert_quad("FUN", temp, name, "--");
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
            char *temp;
            char *name;
            char *scope;
            if (t->child[0] != NULL) {
                temp = type_to_string(t->child[0]);
                name = t->attr.name;
                scope = t->scope;
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
            if (is_asn && is_const_asn && i == 1) {
                continue; // É do tipo `Id`, não há necessidade de continuar
            }

            if (is_iff && i == 1) { // Antes de entrar no `if`
                char *temp = cs_pop(temps);
                char *label = malloc(STRING_SIZE * sizeof(*label));

                snprintf(label, STRING_SIZE, ".LB%d", lab_num);
                insert_quad("IFF", temp, label, "--");

                lab_num++;
                cs_push(ifs, label);
            }

            if (is_while && i == 1) { // Antes de entrar no `while`
                char *temp = cs_pop(temps);
                char *label = malloc(STRING_SIZE * sizeof(*label));

                snprintf(label, STRING_SIZE, ".LB%d", lab_num);
                insert_quad("IFF", temp, label, "--");

                lab_num++;
                cs_push(whiles, label);
            }

            // Chamada dos filhos do nó
            if (is_curr_callk) { // CallK que chama
                is_inside_param = TRUE;
                read_tree_node(t->child[i]);
            } else if (is_inside_param) { // Filhos de CallK chamam
                is_inside_param = FALSE;
                read_tree_node(t->child[i]);
                is_inside_param = TRUE;
            } else { // Outros nós chamam
                read_tree_node(t->child[i]);
            }

            if (is_iff && i == 1) { // Saindo do `if`, entrando no `else`
                // Como há uma comparação se é `NULL` no começo do `for`
                // então não tem como realizar estes passos
                // durante a entrada no `else`
                if (t->child[2] != NULL) { // Tem `else`, fazer pop() -> push()
                    char *l1 = cs_pop(ifs);
                    char *l2 = malloc(STRING_SIZE * sizeof(*l2));

                    snprintf(l2, STRING_SIZE, ".LB%d", lab_num);
                    insert_quad("GOTO", l2, "--", "--");
                    insert_quad("LAB", l1, "--", "--");

                    lab_num++;
                    cs_push(ifs, l2);
                } else { // Não tem else, apenas fazer pop()
                    char *label = cs_pop(ifs);
                    insert_quad("LAB", label, "--", "--");
                }
            } else if (is_iff && i == 2) { // Saindo do `else`
                // Como acontece uma comparação com `NULL` no começo do `for`
                // esse caso só acontece se existe um `else`
                // então não é necessário verificar se é NULL
                char *label = cs_pop(ifs);
                insert_quad("LAB", label, "--", "--");
            }

            if (is_while && i == 1) { // Saindo do `while`
                char *l2 = cs_pop(whiles);
                char *l1 = cs_pop(whiles);

                insert_quad("GOTO", l1, "--", "--");
                insert_quad("LAB", l2, "--", "--");
            }

            if (is_arr) {
                char *name = malloc(STRING_SIZE * sizeof(*name));
                char *temp = malloc(STRING_SIZE * sizeof(*temp));
                char *offset = malloc(STRING_SIZE * sizeof(*offset));
                snprintf(temp, STRING_SIZE, "$t%d", temp_num);
                strcpy(offset, temps->items[temps->last]);
                strcpy(name, t->attr.name);
                insert_quad("ARRLOAD", temp, name, offset);
                temp_num++;
                cs_push(temps, temp);
            }
        }
    }

    if (is_fun) {
        insert_quad("END", t->attr.name, "--", "--");
    }

    if (is_asn) {
        if (is_const_asn) {
            char *temp = cs_pop(temps);
            char *val = malloc(STRING_SIZE * sizeof(*val));
            snprintf(val, STRING_SIZE, "%d", t->child[1]->attr.val);
            if (is_arr_asn) {
                char *offset = cs_pop(temps);
                insert_quad("ASSIGN", temp, val, "--");
                insert_quad("ARRSTR", t->child[0]->attr.name, temp, offset);
            } else {
                insert_quad("ASSIGN", temp, val, "--");
                insert_quad("STORE", t->child[0]->attr.name, temp, "--");
            }
        } else {
            char *t1 = cs_pop(temps);
            char *t2 = cs_pop(temps);
            if (is_arr_asn) {
                char *offset = cs_pop(temps);
                insert_quad("ASSIGN", t2, t1, "--");
                insert_quad("ARRSTR", t->child[0]->attr.name, t2, offset);
            } else {
                insert_quad("ASSIGN", t2, t1, "--");
                insert_quad("STORE", t->child[0]->attr.name, t2, "--");
            }
        }
    }

    if (is_inside_param && !(t->node_kind == ExpK && t->kind.exp == CalcK) &&
        t->sibling != NULL) {
        char *temp = cs_pop(temps);
        insert_quad("PARAM", temp, "--", "--");
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
            snprintf(t3, STRING_SIZE, "$t%d", temp_num);
            temp_num++;
            cs_push(temps, t3);
            insert_quad(cmd, t3, t2, t1);
        } break;
        case ArrIdK:
            is_arr = TRUE;
            break;
        case CallK: {
            if (is_inside_param &&
                !(t->node_kind == ExpK && t->kind.exp == CalcK)) {
                if (t->node_kind == ExpK && t->kind.exp == CallK) {
                    if (nested_call_level <= 0) {
                        is_inside_param = FALSE;
                    }
                }
                char *temp = cs_pop(temps);
                insert_quad("PARAM", temp, "--", "--");
            }
            is_curr_callk = FALSE;
            nested_call_level--;
            if (nested_call_level <= 0) { // Ultima call de todas
                nested_call_level = 0;
                is_inside_param = FALSE;
                is_call = FALSE;
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
            snprintf(temp, STRING_SIZE, "$t%d", temp_num);
            cs_push(temps, temp);
            snprintf(p_num, STRING_SIZE, "%d", param_num);
            insert_quad("CALL", temp, t->attr.name, p_num);

            temp_num++;
        } break;

        case IdK:
            break;

        default:
            break;
        }
    } break;
    case StmtK: {
        switch (t->kind.stmt) {
        case ReturnK: {
            char *temp = cs_pop(temps);
            insert_quad("RET", temp, "--", "--");
        } break;
        default:
            break;
        }
    } break;

    case DeclK: {
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
