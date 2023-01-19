/* Implementação do ódigo intermediário */

#include "intermediate.h"

#include <stdbool.h>

#include "common/globals.h"
#include "common/util.h"

/* Tamanho máximo de uma string */
#define STRING_SIZE 64

/* Quádrupla final */
static Quad quad;

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
static int inside_param = false;

/* Não é necessário guardar o offset se ele não faz parte do lado esquerdo
 * de um assign, já que não vai acontecer um ARRSTR com esse offset
 */
static int arr_is_left_assign = true;

/* Função `print_quad` printa a quadrupla inteira */
static void
print_quad(Quad q)
{
    Quad current = q;

    fprintf(std_fd, "|%7s, %7s, %7s, %7s|\n\n", "CMD", "ARG1", "ARG2", "ARG3");

    while (current != NULL) {
        fprintf(
            std_fd, "(%7s, %7s, %7s, %7s)\n", current->cmd, current->arg1,
            current->arg2, current->arg3
        );

        current = current->next;
    }
}

/* Função `insert_quad` insere quadruplas na variável quad */
static void
insert_quad(char *cmd, char *arg1, char *arg2, char *arg3)
{
    Quad new_quad = malloc_or_die(sizeof(*new_quad));

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
static char *
type_to_string(TreeNode *t)
{
    switch (t->type) {
    case Integer:
        return "INT";
    case IntegerArray:
        return "INTARR";
    case Void:
        return "VOID";
    default: // Não deve acontecer normalmente
        return NULL;
    }
}

/* Função `new_label` retorna o próximo label disponível */
static char *
new_label(void)
{
    static int available_num = 0;
    char *label = malloc_or_die(STRING_SIZE * sizeof(*label));

    snprintf(label, STRING_SIZE, ".LB%d", available_num);
    available_num += 1;

    return label;
}

/* Função `new_temp` retorna o próximo temporário disponível */
static char *
new_temp(void)
{
    static int available_num = 0;
    char *temp = malloc_or_die(STRING_SIZE * sizeof(*temp));

    snprintf(temp, STRING_SIZE, "$t%d", available_num);
    available_num += 1;

    return temp;
}

/* Função `read_tree_node` lê o nó da árvore e trata ele de acordo */
/* TODO: tratar todos os casos possíveis */
static void
read_tree_node(TreeNode *t)
{
    int fun_node = false;
    int assign_node = false;
    int const_assign_node = false;
    int inside_call = nested_call_level <= 0 ? false : true;
    int if_node = false;
    int while_node = false;
    int array_node = false;
    int array_assign_node = false;
    int callk_node = false;

    // Verifica o nó atual
    switch (t->node_kind) {
    case StmtK:
        switch (t->kind.stmt) {
        case IfK:
            if_node = true;
            break;
        case WhileK: {
            char *lab = new_label();

            insert_quad("LAB", lab, "--", "--");
            cs_push(whiles, lab);

            while_node = true;
        } break;
        case AssignK:
            assign_node = true;
            if (t->child[0]->node_kind == ExpK &&
                t->child[0]->kind.exp == ArrIdK) {
                array_assign_node = true;
            }

            if (t->child[1]->node_kind == ExpK &&
                t->child[1]->kind.exp == ConstK) {
                const_assign_node = true;
            }
            break;
        default:
            break;
        }
        break;
    case ExpK: {
        switch (t->kind.exp) {
        case ConstK: {
            char *temp = new_temp();
            char *value = malloc_or_die(STRING_SIZE * sizeof(*value));

            snprintf(value, STRING_SIZE, "%d", t->attr.val);
            cs_push(temps, temp);
            insert_quad("LOAD", temp, value, "--");
        } break;
        case IdK: {
            if (inside_call || !fun_node) {
                char *temp = new_temp();

                cs_push(temps, temp);
                insert_quad("LOAD", temp, t->attr.name, "--");
            }
        } break;
        case ArrIdK:
            array_node = true;
            break;
        case CallK:
            callk_node = true;
            inside_call = true;
            nested_call_level++;
            break;
        default:
            break;
        }
    } break;
    case DeclK:
        switch (t->kind.decl) {
        case VarK:
            insert_quad("ALLOC", t->attr.name, t->scope, "--");
            break;
        case FunK:
            insert_quad("FUN", type_to_string(t->child[0]), t->attr.name, "--");
            fun_node = true;
            break;
        case ArrVarK: {
            char *name = t->attr.arr.name;
            char *size = malloc_or_die(STRING_SIZE * sizeof(*size));

            snprintf(size, STRING_SIZE, "%d", t->attr.arr.size);
            insert_quad("ARRLOC", name, t->scope, size);
        } break;
        case ParamK: {
            if (t->child[0] != NULL) {
                char *temp = type_to_string(t->child[0]);
                char *name = t->attr.name;
                char *scope = t->scope;

                insert_quad("ARG", temp, name, scope);
            }
        } break;
        default:
            break;
        }
        break;
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

            if (assign_node && i == 0) { // Array é na esquerda
                arr_is_left_assign = true;
            } else if (assign_node && i != 0) { // Array é na direita
                arr_is_left_assign = false;
            }

            if (if_node && i == 1) { // Antes de entrar no `if`
                char *temp = cs_pop(temps);
                char *label = new_label();

                insert_quad("IFF", temp, label, "--");
                cs_push(ifs, label);
            }

            if (while_node && i == 1) { // Antes de entrar no `while`
                char *temp = cs_pop(temps);
                char *label = new_label();

                insert_quad("IFF", temp, label, "--");
                cs_push(whiles, label);
            }

            // Chamada dos filhos do nó
            if (callk_node) { // CallK que chama
                inside_param = true;
                read_tree_node(t->child[i]);
            } else if (inside_param) { // Filhos de CallK chamam
                inside_param = false;
                read_tree_node(t->child[i]);
                inside_param = true;
            } else { // Outros nós chamam
                read_tree_node(t->child[i]);
            }

            if (if_node && i == 1) { // Saindo do `if`, entrando no `else`
                // Como há uma comparação se é `NULL` no começo do `for`
                // então não tem como realizar estes passos
                // durante a entrada no `else`
                if (t->child[2] != NULL) { // Tem `else`, fazer pop() -> push()
                    char *l1 = cs_pop(ifs);
                    char *l2 = new_label();

                    insert_quad("GOTO", l2, "--", "--");
                    insert_quad("LAB", l1, "--", "--");

                    cs_push(ifs, l2);
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
                char *name = copy_string(t->attr.name);
                char *temp = new_temp();
                char *offset;

                if (arr_is_left_assign == false) {
                    offset = cs_pop(temps); // Não é pra guardar o offset
                } else {
                    offset = copy_string(cs_peek(temps));
                }

                insert_quad("ARRLOAD", temp, name, offset);
                cs_push(temps, temp);
            }
        }
    }

    if (fun_node) {
        insert_quad("END", t->attr.name, "--", "--");
    }

    if (assign_node) {
        if (const_assign_node) {
            char *temp = cs_pop(temps);
            char *val = malloc_or_die(STRING_SIZE * sizeof(*val));
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

    if (t->node_kind == ExpK && t->kind.exp == CallK) {
        if (inside_param && !(t->node_kind == ExpK && t->kind.exp == CalcK)) {
            if (t->node_kind == ExpK && t->kind.exp == CallK) {
                if (nested_call_level <= 0) {
                    inside_param = false;
                }
            }

            insert_quad("PARAM", cs_pop(temps), "--", "--");
        }

        callk_node = false;
        nested_call_level--;

        if (nested_call_level <= 0) { // Ultima call de todas
            nested_call_level = 0;
            inside_param = false;
            inside_call = false;
        }

        // Calculando quantos parâmetros a função tem
        int param_num = 0;
        TreeNode *curr_node = t->child[0];

        while (curr_node != NULL) {
            curr_node = curr_node->sibling;
            param_num++;
        }

        if ((strcmp(t->attr.name, "output") == 0) ||
            (strcmp(t->attr.name, "save_reg") == 0) ||
            (strcmp(t->attr.name, "load_reg") == 0) ||
            (strcmp(t->attr.name, "set_preempt") == 0) ||
            (strcmp(t->attr.name, "set_pc") == 0)) {
            char *p_num = malloc_or_die(STRING_SIZE * sizeof(*p_num));

            snprintf(p_num, STRING_SIZE, "%d", param_num);
            insert_quad("CALL", "--", t->attr.name, p_num);
        } else {
            char *temp = new_temp();
            char *p_num = malloc_or_die(STRING_SIZE * sizeof(*p_num));

            cs_push(temps, temp);
            snprintf(p_num, STRING_SIZE, "%d", param_num);
            insert_quad("CALL", temp, t->attr.name, p_num);
        }
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
        case OpK:
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
            break;
        case CalcK: {
            char *t1 = cs_pop(temps);
            char *t2 = cs_pop(temps);
            char *t3 = new_temp();
            char *cmd = cs_pop(calcs);

            cs_push(temps, t3);
            insert_quad(cmd, t3, t2, t1);
        } break;
        case ArrIdK:
            array_node = true;
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
    default:
        break;
    }
}

/* Função `make_intermediate` é responsável por gerar o código intermediário */
Quad
make_intermediate(TreeNode *t)
{
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
