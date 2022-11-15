/* Implementação das funções utilitárias */

#include "util.h"
#include "globals.h"

/* Printando os tokens e lexemas */
void print_token(TokenType token, const char *token_string) {
    switch (token) {
    case IF:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case ELSE:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case INT:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case VOID:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case RETURN:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case WHILE:
        fprintf(listing, "Palavra reservada: %s\n", token_string);
        break;
    case PLUS:
        fprintf(listing, "+\n");
        break;
    case MINUS:
        fprintf(listing, "-\n");
        break;
    case TIMES:
        fprintf(listing, "*\n");
        break;
    case OVER:
        fprintf(listing, "/\n");
        break;
    case LT:
        fprintf(listing, "<\n");
        break;
    case LE:
        fprintf(listing, "<=\n");
        break;
    case GT:
        fprintf(listing, ">\n");
        break;
    case GE:
        fprintf(listing, ">=\n");
        break;
    case EQ:
        fprintf(listing, "==\n");
        break;
    case NE:
        fprintf(listing, "!=\n");
        break;
    case ASSIGN:
        fprintf(listing, "=\n");
        break;
    case SEMI:
        fprintf(listing, ";\n");
        break;
    case COMMA:
        fprintf(listing, ",\n");
        break;
    case LPAREN:
        fprintf(listing, "(\n");
        break;
    case RPAREN:
        fprintf(listing, ")\n");
        break;
    case LBRACKET:
        fprintf(listing, "[\n");
        break;
    case RBRACKET:
        fprintf(listing, "]\n");
        break;
    case LBRACE:
        fprintf(listing, "{\n");
        break;
    case RBRACE:
        fprintf(listing, "}\n");
        break;
    case ENDFILE:
        fprintf(listing, "%s %s\n", "ENDFILE", "EOF");
        break;
    case NUM:
        fprintf(listing, "NUM, val = %s\n", token_string);
        break;
    case ID:
        fprintf(listing, "ID, nome = %s\n", token_string);
        break;
    case ERROR:
        fprintf(listing, "ERRO LÉXICO: %s LINHA: %d\n", token_string, lineno);
        g_error = true;
        break;
    default: // Não deve acontecer normalmente
        fprintf(listing, "Token desconhecido: %d\n", token);
        break;
    }
}

/* Função `new_StmtNode` cria um novo nó do tipo statement
 * para a contrução da árvore sintática
 */
TreeNode *new_StmtNode(StmtKind kind) {
    TreeNode *t = malloc(sizeof(*t));

    if (t == NULL) {
        fprintf(listing, "Sem memória na linha %d\n", lineno);
    } else {
        for (int i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node_kind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
    }

    return t;
}

/* Função `new_ExpNode` cria um novo nó do tipo expressão
 * para a contrução da árvore sintática
 */
TreeNode *new_ExpNode(ExpKind kind) {
    TreeNode *t = malloc(sizeof(*t));

    if (t == NULL) {
        fprintf(listing, "Erro de falta de memória na linha %d\n", lineno);
    } else {
        for (int i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node_kind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
    }

    return t;
}

/* Função `new_DeclNode` cria um novo nó do tipo declaração
 * para a contrução da árvore sintática
 */
TreeNode *new_DeclNode(DeclKind kind) {
    TreeNode *t = malloc(sizeof(*t));

    if (t == NULL) {
        fprintf(listing, "Erro de falta de momória na linha %d\n", lineno);
    } else {
        for (int i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node_kind = DeclK;
        t->kind.decl = kind;
        t->lineno = lineno;
        t->type = Void;
    }

    return t;
}

/* Função `copy_string` aloca e cria uma nova
 * cópia de uma string existente
 */
char *copy_string(char *s) {
    int n;
    char *t;
    if (s == NULL) {
        return NULL;
    }
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL) {
        fprintf(listing, "Erro de falta de memória na linha %d\n", lineno);
    } else {
        strcpy(t, s);
    }

    return t;
}

/* Variável `indentno` é usada pelo print_tree para
 * guardar o número atual da indentação
 */
static int indentno = 0;

/* Macros para aumentar/diminuir a indentação */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* Função `print_spaces` indenta printando espaços */
static void print_spaces(void) {
    for (int i = 0; i < indentno; i++) {
        fprintf(listing, " ");
    }
}

/* Função `print_types` printa os tipos de funções e variáveis */
void print_types(TreeNode *tree) {
    if (tree->child[0] != NULL) {
        switch (tree->child[0]->type) {
        case Integer:
            fprintf(listing, "int");
            break;
        case Void:
            fprintf(listing, "void");
            break;
        case IntegerArray:
            fprintf(listing, "int array");
            break;
        default:
            return;
        }
    } else {
        switch (tree->type) {
        case Integer:
            fprintf(listing, "int");
            break;
        case Void:
            fprintf(listing, "void");
            break;
        case IntegerArray:
            fprintf(listing, "int array");
            break;
        default:
            return;
        }
    }
}

/* Função `print_tree` printa a árvore sintática para o
 * arquivo listing usando indentação para indicar sub-árvores
 */
void print_tree(TreeNode *tree) {
    INDENT;
    while (tree != NULL) {
        print_spaces();

        // IfK, WhileK, CompoundK, ReturnK
        if (tree->node_kind == StmtK) {
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(listing, "If\n");
                break;
            case WhileK:
                fprintf(listing, "While\n");
                break;
            case CompoundK:
                fprintf(listing, "Declaração composta\n");
                break;
            case ReturnK:
                fprintf(listing, "Return\n");
                break;
            case AssignK:
                fprintf(listing, "Atribuição:\n");
                break;
            default:
                fprintf(listing, "Tipo de declaração desconhecida\n");
                break;
            }
        }
        // OpK, ConstK, AssignK, IdK, TypeK, ArrIdK, CallK, CalcK
        else if (tree->node_kind == ExpK) {
            if (tree->kind.exp != TypeK)
                switch (tree->kind.exp) {
                case OpK:
                    fprintf(listing, "Op: ");
                    print_token(tree->attr.op, "\0");
                    break;
                case ConstK:
                    fprintf(listing, "Const: %d\n", tree->attr.val);
                    break;
                case IdK:
                    fprintf(listing, "Id: %s\n", tree->attr.name);
                    break;
                case TypeK:
                    break;
                case ArrIdK:
                    fprintf(listing, "ArrId: %s\n", tree->attr.name);
                    break;
                case CallK:
                    fprintf(listing, "Chamada de Função: %s\n",
                            tree->attr.name);
                    break;
                case CalcK:
                    fprintf(listing, "Operador: ");
                    print_token(tree->child[1]->attr.op, "\0");
                    break;
                default:
                    fprintf(listing, "Tipo de expressão desconhecida\n");
                    break;
                }
        }
        // VarK, FunK, ArrVarK, ArrParamK, ParamK
        else if (tree->node_kind == DeclK) {
            switch (tree->kind.decl) {
            case FunK:
                fprintf(listing, "Declaração de Função:  ");
                print_types(tree);
                fprintf(listing, " %s()\n", tree->attr.name);
                break;
            case VarK:
                fprintf(listing, "Declaração de variável:  ");
                print_types(tree);
                fprintf(listing, " %s;\n", tree->attr.name);
                break;
            case ArrVarK:
                fprintf(listing, "Declaração de array de variável:  ");
                print_types(tree);
                fprintf(listing, " %s[%d];\n", tree->attr.arr.name,
                        tree->attr.arr.size);
                break;
            case ArrParamK:
                fprintf(listing, "Parâmetro de array: %s\n", tree->attr.name);
                break;
            case ParamK:
                fprintf(listing, "Parâmetro: ");
                print_types(tree);
                if (tree->attr.name != NULL) {
                    fprintf(listing, " %s\n", tree->attr.name);
                } else {
                    fprintf(listing, " void\n");
                }
                break;
            default:
                fprintf(listing, "Declaração desconhecida\n");
                break;
            }
        } else {
            fprintf(listing, "Tipo de nó desconhecido\n");
        }

        for (int i = 0; i < MAXCHILDREN; i++) {
            if (tree->child[i] != NULL) {
                print_tree(tree->child[i]);
            }
        }

        tree = tree->sibling;
    }
    UNINDENT;
}

/* Função `cs_init` inicia a pilha com um tamanho máximo de `size` */
CharStack cs_init(void) {
    int size = 256;

    CharStack new = malloc(sizeof(*new));
    new->max_size = size;
    new->last = -1;
    new->items = malloc(size * sizeof(*new->items));

    return new;
}

/* Função `cs_push` coloca a string na pilha */
void cs_push(CharStack stack, char *item) {
    if (stack->last >= stack->max_size) { // Redimencionar
        stack->max_size *= 2;
        stack->items =
            realloc(stack->items, stack->max_size * sizeof(**stack->items));
    }

    stack->last += 1;
    stack->items[stack->last] = malloc(strlen(item) * sizeof(item));
    strcpy(stack->items[stack->last], item);
}

/* Função `cs_pop` remove a string da pilha, retornando seu valor */
char *cs_pop(CharStack stack) {
    char *item = stack->items[stack->last];

    stack->items[stack->last] = NULL;
    stack->last -= 1;

    return item;
}

/* Função `cs_drop` remove a memória usada pela pilha */
void cs_drop(CharStack stack) {
    for (int i = 0; i <= stack->last; i++) {
        free(stack->items[i]);
    }

    free(stack->items);
    free(stack);
}
