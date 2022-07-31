// Código intermediário
// TODO: tratar while
// TODO: tratar array

#include "globals.h"
#include "code.h"

// TODO: `STACK_SIZE` é um limite arbitrário, logo se possível mudar
//   @t! para uma implementação sem limite, provavelmente através de realloc
#define STACK_SIZE 50

/* Quadrupla final */
static Quad quad;

/* Número de $t e L */
static int t_num = 0;
static int l_num = 0;

// TODO: talvez seja melhor abstrair o conceito de `stack` para um struct
//   @t! possivelmente implementar esse struct melhore a clareza do código
/* Ordem de LOAD */
static char stack_load[STACK_SIZE][50];
static int stack_load_last = 0;

/* Ordem de $t */
static char stack_temp[STACK_SIZE][50];
static int stack_temp_last = 0;

/* Ordem de calculos */
static char stack_calc[STACK_SIZE][50];
static int stack_calc_last = 0;

/* O quanto dentro de uma call o programa está */
static int nested_call_level = 0;

/* Função print_quad printa a quadrupla inteira */
static void print_quad(Quad q) {
    Quad curr_q = q;
    fprintf(listing, "|%7s, %7s, %7s, %7s|\n\n", "CMD", "ARG1", "ARG2", "ARG3");
    while (curr_q != NULL) {
        fprintf(listing, "(%7s, %7s, %7s, %7s)\n", curr_q->command,
                curr_q->arg1, curr_q->arg2, curr_q->arg3);
        curr_q = curr_q->next;
    }
}

/* Função push insere um elemento no stack */
void push(char stack[STACK_SIZE][50], char *elem, int *stack_size) {
    if (*stack_size >= STACK_SIZE) {
        Error = 1;
        exit(1);
    }

    strcpy(stack[*stack_size], elem);
    *stack_size = *stack_size + 1;
}

/* Função pop remove um elemento do stack */
void pop(char stack[STACK_SIZE][50], int *stack_size) {
    *stack_size = *stack_size - 1;
    if (*stack_size < 0) {
        Error = 1;
        exit(1);
    }

    strcpy(stack[*stack_size], "\0");
}

/* Função insert_quad insere quadruplas na variável quad */
static void insert_quad(char *cmd, char *arg1, char *arg2, char *arg3) {
    Quad new_quad = (Quad)malloc(sizeof(struct QuadRec));
    new_quad->command = cmd;
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

/* Função type_to_string transforma o tipo em uma string, void ou int */
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
    }
}

/* Função read_tree_node lê o nó da arvore e trata ele de acordo */
/* TODO: tratar todos os casos possíveis */
static void read_tree_node(TreeNode *t) {
    // TODO: melhorar e documentar

    // TODO: remover a maioria do código
    //   @t! transformando read_tree_node em uma função que executa 3 outras
    //   @t! sendo elas uma pré, uma mid e uma pós nó
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
    static int is_inside_param = FALSE;
    static int is_inside_while = FALSE;
    if (nested_call_level <= 0) {
        is_call = FALSE;
    } else {
        is_call = TRUE;
    }

    // Verifica o nó atual
    switch (t->nodekind) {
    case StmtK: {
        switch (t->kind.stmt) {
        case IfK:
            is_iff = TRUE;
            break;
        case WhileK: {
            char *l1 = (char *)malloc(50 * sizeof(char));
            snprintf(l1, 50, "L%d", l_num);
            insert_quad("LAB", l1, "--", "--");
            l_num++;
            is_inside_while = TRUE;
            is_while = TRUE;
        } break;
        case AssignK:
            is_asn = TRUE;
            if (t->child[0]->nodekind == ExpK &&
                t->child[0]->kind.exp == ArrIdK) {
                is_arr_asn = TRUE;
            }

            if (t->child[1]->nodekind == ExpK &&
                t->child[1]->kind.exp == ConstK) {
                is_const_asn = TRUE;
            }
            break;
        case CompoundK:
            /* code */
            break;
        case ReturnK:
            break;
        }
        break;
    }
    case ExpK: {
        switch (t->kind.exp) {
        case OpK:
            switch (t->attr.op) {
            case LT:
                break;

            case LE:
                break;

            case GT:
                break;

            case GE:
                break;

            case EQ:
                break;

            case NE:
                break;

            case PLUS:
                break;

            case MINUS:
                break;

            case TIMES:
                break;

            case OVER:
                break;
            }
            break;

        case ConstK: {
            char *temp = (char *)malloc(50 * sizeof(char));
            char *c = (char *)malloc(50 * sizeof(char));
            snprintf(c, 50, "%d", t->attr.val);
            snprintf(temp, 50, "$t%d", t_num);
            push(stack_temp, temp, &stack_temp_last);
            insert_quad("LOAD", temp, c, "--");
            t_num++;
            break;
        }
        case IdK: {
            if (is_call || !is_fun) {
                char *temp = (char *)malloc(50 * sizeof(char));
                char *c = (char *)malloc(50 * sizeof(char));
                snprintf(temp, 50, "$t%d", t_num);
                push(stack_temp, temp, &stack_temp_last);
                t_num++;
                insert_quad("LOAD", temp, t->attr.name, "--");
            }
            break;
        }
        case TypeK:
            /* code */
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
        break;
    }
    case DeclK: {
        switch (t->kind.decl) {
        case VarK: {
            char *name;
            char *scope;
            name = t->attr.name;
            insert_quad("ALLOC", name, t->scope, "--");
            break;
        }
        case FunK: {
            char *name;
            char *temp = (char *)malloc(50 * sizeof(char));
            char *c = (char *)malloc(50 * sizeof(char));
            is_fun = TRUE;
            temp = type_to_string(t->child[0]);
            name = t->attr.name;
            insert_quad("FUN", temp, name, "--");
            break;
        }
        case ArrVarK: {
            char *name;
            char *scope;
            char *size = malloc(50 * sizeof size);
            name = t->attr.arr.name;
            snprintf(size, 50, "%d", t->attr.arr.size);
            insert_quad("ARRLOC", name, t->scope, size);
            break;
        }
        case ArrParamK:
            /* code */
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
            break;
        } break;
        }
    }
    }

    // Verifica os 3 filhos
    for (int i = 0; i < 3; i++) {
        if (t->child[i] != NULL) {
            if (t->child[i]->nodekind == ExpK &&
                t->child[i]->kind.exp == TypeK) {
                continue; // Type
            }
            if (is_asn && is_const_asn && i == 1) {
                continue; // Id
            }
            if ((i == 1 && is_iff) || (i == 1 && is_while)) {
                char *t = (char *)malloc(50 * sizeof(char));
                char *l = (char *)malloc(50 * sizeof(char));
                strcpy(t, stack_temp[stack_temp_last - 1]);
                snprintf(l, 50, "L%d", l_num);
                l_num++;
                pop(stack_temp, &stack_temp_last);
                insert_quad("IFF", t, l, "--");
            }

            // if (i == 2 && is_fun) { // Todos parâmetros lidos
            //     while (stack_load_last > 0) {
            //         char *t = (char *)malloc(50 * sizeof(char));
            //         char *c = (char *)malloc(50 * sizeof(char));
            //         strcpy(c, stack_load[stack_load_last - 1]);
            //         snprintf(t, 50, "$t%d", t_num);
            //         push(stack_temp, t, &stack_temp_last);
            //         t_num++;
            //         insert_quad("LOAD", t, c, "--");
            //         pop(stack_load, &stack_load_last);
            //     }
            // }

            if (is_curr_callk) { // CallK que chama
                is_inside_param = TRUE;
                read_tree_node(t->child[i]);
            } else if (is_inside_param) { // Filhos de CallK chamam
                is_inside_param = FALSE;
                read_tree_node(t->child[i]);
                is_inside_param = TRUE;
            } else {
                read_tree_node(t->child[i]);
            }

            if (i == 1 && is_iff) {
                char *l1 = (char *)malloc(50 * sizeof(char));
                char *l2 = (char *)malloc(50 * sizeof(char));
                snprintf(l1, 50, "L%d", l_num);
                insert_quad("GOTO", l1, "--", "--");
                snprintf(l2, 50, "L%d", l_num - 1); // Primeiro L do if atual
                insert_quad("LAB", l2, "--", "--");
            } else if (i == 2 && is_iff) {
                char *l1 = (char *)malloc(50 * sizeof(char));
                char *l2 = (char *)malloc(50 * sizeof(char));
                snprintf(l1, 50, "L%d", l_num);
                insert_quad("GOTO", l1, "--", "--");
                snprintf(l2, 50, "L%d", l_num);
                insert_quad("LAB", l2, "--", "--");
                l_num++;
                if (is_inside_while) {
                    l_num -= 2;
                }
            }

            if (i == 1 && is_while) {
                char *l1 = (char *)malloc(50 * sizeof(char));
                char *l2 = (char *)malloc(50 * sizeof(char));
                snprintf(l1, 50, "L%d", l_num - 2);
                insert_quad("GOTO", l1, "--", "--");
                snprintf(l2, 50, "L%d", l_num - 1); // Primeiro L do if atual
                insert_quad("LAB", l2, "--", "--");
            }

            if (is_arr) {
                char *name = malloc(50 * sizeof name);
                char *temp = malloc(50 * sizeof temp);
                char *offset = malloc(50 * sizeof offset);
                snprintf(temp, 50, "$t%d", t_num);
                strcpy(offset, stack_temp[stack_temp_last - 1]);
                strcpy(name, t->attr.name);
                insert_quad("ARRLOAD", temp, name, offset);
                push(stack_temp, temp, &stack_temp_last);
            }
        }
    }

    if (is_fun) {
        insert_quad("END", t->attr.name, "--", "--");
    }

    if (is_asn) {
        if (is_const_asn) {
            char *temp = (char *)malloc(50 * sizeof(char));
            strcpy(temp, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            char *val = (char *)malloc(50 * sizeof(char));
            snprintf(val, 50, "%d", t->child[1]->attr.val);
            if (is_arr_asn) {
                char *offset = malloc(50 * sizeof offset);
                strcpy(offset, stack_temp[stack_temp_last - 1]);
                pop(stack_temp, &stack_temp_last);
                insert_quad("ASSIGN", temp, val, "--");
                insert_quad("ARRSTR", t->child[0]->attr.name, temp, offset);
            } else {
                insert_quad("ASSIGN", temp, val, "--");
                insert_quad("STORE", t->child[0]->attr.name, temp, "--");
            }
        } else {
            char *t1 = (char *)malloc(50 * sizeof(char));
            strcpy(t1, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            char *t2 = (char *)malloc(50 * sizeof(char));
            strcpy(t2, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            if (is_arr_asn) {
                char *offset = malloc(50 * sizeof offset);
                strcpy(offset, stack_temp[stack_temp_last - 2]);
                pop(stack_temp, &stack_temp_last);
                insert_quad("ASSIGN", t2, t1, "--");
                insert_quad("ARRSTR", t->child[0]->attr.name, t2, offset);
            } else {
                insert_quad("ASSIGN", t2, t1, "--");
                insert_quad("STORE", t->child[0]->attr.name, t2, "--");
            }
        }
    }

    if (is_inside_param && !(t->nodekind == ExpK && t->kind.exp == CalcK) &&
        t->sibling != NULL) {
        char *temp = (char *)malloc(50 * sizeof(char));
        strcpy(temp, stack_temp[stack_temp_last - 1]);
        pop(stack_temp, &stack_temp_last);
        insert_quad("PARAM", temp, "--", "--");
    }

    // Verifica o irmão
    if (t->sibling != NULL) {
        read_tree_node(t->sibling);
    }

    // Adiciona parâmetro no stack
    if (t->nodekind == DeclK && t->kind.decl == ParamK && t->child[0] != NULL) {
        push(stack_load, t->attr.name, &stack_load_last);
    }

    switch (t->nodekind) {
    case ExpK: {
        switch (t->kind.exp) {
        case OpK: {
            switch (t->attr.op) {
            case LT: {
                push(stack_calc, "LESS", &stack_calc_last);
            } break;
            case LE: {
                push(stack_calc, "LEQ", &stack_calc_last);
            } break;
            case GT: {
                push(stack_calc, "GREAT", &stack_calc_last);
            } break;
            case GE: {
                push(stack_calc, "GRTEQ", &stack_calc_last);
            } break;
            case EQ: {
                push(stack_calc, "EQUAL", &stack_calc_last);
            } break;
            case NE: {
                push(stack_calc, "NOTEQ", &stack_calc_last);
            } break;
            case PLUS:
                push(stack_calc, "ADD", &stack_calc_last);
                break;
            case MINUS:
                push(stack_calc, "SUB", &stack_calc_last);
                break;
            case TIMES:
                push(stack_calc, "MULT", &stack_calc_last);
                break;
            case OVER:
                push(stack_calc, "DIV", &stack_calc_last);
                break;
            }
            break;
        }
        case CalcK: {
            char *t1 = (char *)malloc(50 * sizeof(char));
            strcpy(t1, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            char *t2 = (char *)malloc(50 * sizeof(char));
            strcpy(t2, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            char *t3 = (char *)malloc(50 * sizeof(char));
            snprintf(t3, 50, "$t%d", t_num);
            t_num++;
            push(stack_temp, t3, &stack_temp_last);
            char *cmd = (char *)malloc(50 * sizeof(char));
            strcpy(cmd, stack_calc[stack_calc_last - 1]);
            pop(stack_calc, &stack_calc_last);
            insert_quad(cmd, t3, t2, t1);
            // if (is_inside_param) {
            //     char *temp = (char *)malloc(50 * sizeof(char));
            //     strcpy(temp, stack_temp[stack_temp_last - 1]);
            //     pop(stack_temp, &stack_temp_last);
            //     insert_quad("PARAM", temp, "--", "--");
            // }
            break;
        }
        case ArrIdK:
            is_arr = TRUE;
            break;
        case CallK: {
            if (is_inside_param &&
                !(t->nodekind == ExpK && t->kind.exp == CalcK)) {
                if (t->nodekind == ExpK && t->kind.exp == CallK) {
                    if (nested_call_level <= 0) {
                        is_inside_param = FALSE;
                    }
                }
                char *temp = (char *)malloc(50 * sizeof(char));
                strcpy(temp, stack_temp[stack_temp_last - 1]);
                pop(stack_temp, &stack_temp_last);
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
            char *temp = (char *)malloc(50 * sizeof(char));
            char *p_num = (char *)malloc(50 * sizeof(char));
            snprintf(temp, 50, "$t%d", t_num);
            push(stack_temp, temp, &stack_temp_last);
            snprintf(p_num, 50, "%d", param_num);
            insert_quad("CALL", temp, t->attr.name, p_num);

            t_num++;
            break;
        }
        case IdK:
            break;
        default:
            break;
        }
        break;
    }
    case StmtK: {
        switch (t->kind.stmt) {
        case ReturnK: {
            char *temp = (char *)malloc(50 * sizeof(char));
            strcpy(temp, stack_temp[stack_temp_last - 1]);
            pop(stack_temp, &stack_temp_last);
            insert_quad("RET", temp, "--", "--");
            break;
        }
        default:
            break;
        }
        break;
    }
    }
}

/* make_code é responsável por gerar o código intermediário */
/* TODO: transformar os nomes das funções e variáveis
 *   @t! de Camel case para Snake case
 */
Quad make_code(TreeNode *t) {

    read_tree_node(t);
    insert_quad("HALT", "--", "--", "--");

    if (TraceCode) {
        print_quad(quad);
    }

    return quad;
}

// TODO: GOTO e LAB na volta do return
