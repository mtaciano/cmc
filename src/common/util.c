/* Implementação das funções utilitárias */

#include "util.h"

#include "globals.h"

/* Função `malloc_or_die` é um _wrapper_ para a função `malloc`
 * com a diferença que, quando não foi possível realizar o `malloc`,
 * ela chama `exit` e encerra a execução do código
 */
// NOTE: o ato de chamar `exit` pode não ser o mais adequado para se tratar
// durante uma falha do `malloc`, há outras alternativas como liberar memória
// e tentar novamente, no entanto essa solução é simples e eficaz para o
// propósito do projeto, então é a implementação atual
void *
malloc_or_die(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL) {
        fprintf(err_fd, "Falta de memória em malloc_or_die().\n");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

/* Função `realloc_or_die` é um _wrapper_ para a função `realloc`
 * com a diferença que, quando não foi possível realizar o `realloc`,
 * ela chama `exit` e encerra a execução do código
 */
// NOTE: o ato de chamar `exit` pode não ser o mais adequado para se tratar
// durante uma falha do `realloc`, há outras alternativas como liberar memória
// e tentar novamente, no entanto essa solução é simples e eficaz para o
// propósito do projeto, então é a implementação atual
void *
realloc_or_die(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);

    if (new_ptr == NULL) {
        fprintf(err_fd, "Falta de memória em realloc_or_die().\n");
        exit(EXIT_FAILURE);
    }

    return new_ptr;
}

/* Função `printToken` printa um token
 * e seu lexema para o arquivo std_fd
 */
void
print_token(TokenType token, const char *token_string)
{
    switch (token) {
    case IF:
        /* fallthrough */
    case ELSE:
        /* fallthrough */
    case INT:
        /* fallthrough */
    case VOID:
        /* fallthrough */
    case RETURN:
        /* fallthrough */
    case WHILE:
        fprintf(std_fd, "Palavra reservada: %s\n", token_string);
        break;
    case PLUS:
        fprintf(std_fd, "+\n");
        break;
    case MINUS:
        fprintf(std_fd, "-\n");
        break;
    case TIMES:
        fprintf(std_fd, "*\n");
        break;
    case OVER:
        fprintf(std_fd, "/\n");
        break;
    case LT:
        fprintf(std_fd, "<\n");
        break;
    case LE:
        fprintf(std_fd, "<=\n");
        break;
    case GT:
        fprintf(std_fd, ">\n");
        break;
    case GE:
        fprintf(std_fd, ">=\n");
        break;
    case EQ:
        fprintf(std_fd, "==\n");
        break;
    case NE:
        fprintf(std_fd, "!=\n");
        break;
    case ASSIGN:
        fprintf(std_fd, "=\n");
        break;
    case SEMI:
        fprintf(std_fd, ";\n");
        break;
    case COMMA:
        fprintf(std_fd, ",\n");
        break;
    case LPAREN:
        fprintf(std_fd, "(\n");
        break;
    case RPAREN:
        fprintf(std_fd, ")\n");
        break;
    case LBRACKET:
        fprintf(std_fd, "[\n");
        break;
    case RBRACKET:
        fprintf(std_fd, "]\n");
        break;
    case LBRACE:
        fprintf(std_fd, "{\n");
        break;
    case RBRACE:
        fprintf(std_fd, "}\n");
        break;
    case ENDFILE:
        fprintf(std_fd, "ENDFILE EOF\n");
        break;
    case NUM:
        fprintf(std_fd, "NUM, valor = %s\n", token_string);
        break;
    case ID:
        fprintf(std_fd, "ID, nome = %s\n", token_string);
        break;
    case ERROR:
        fprintf(err_fd, "ERRO LÉXICO: %s LINHA: %d\n", token_string, lineno);
        exit(EXIT_FAILURE);
    default: // Não deve acontecer normalmente
        fprintf(err_fd, "Token desconhecido: %d\n", token);
        exit(EXIT_FAILURE);
    }
}

/* Função `new_StmtNode` cria um novo nó do tipo statement
 * para a contrução da árvore sintática
 */
TreeNode *
new_StmtNode(StmtKind kind)
{
    TreeNode *t = malloc_or_die(sizeof(*t));

    for (int i = 0; i < MAXCHILDREN; i++) {
        t->child[i] = NULL;
    }
    t->sibling = NULL;
    t->node_kind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;

    return t;
}

/* Função `new_ExpNode` cria um novo nó do tipo expressão
 * para a contrução da árvore sintática
 */
TreeNode *
new_ExpNode(ExpKind kind)
{
    TreeNode *t = malloc_or_die(sizeof(*t));

    for (int i = 0; i < MAXCHILDREN; i++) {
        t->child[i] = NULL;
    }
    t->sibling = NULL;
    t->node_kind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;

    return t;
}

/* Função `new_DeclNode` cria um novo nó do tipo declaração
 * para a contrução da árvore sintática
 */
TreeNode *
new_DeclNode(DeclKind kind)
{
    TreeNode *t = malloc_or_die(sizeof(*t));

    for (int i = 0; i < MAXCHILDREN; i++) {
        t->child[i] = NULL;
    }
    t->sibling = NULL;
    t->node_kind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
    t->type = Void;

    return t;
}

/* Função `copy_string` aloca e cria uma nova
 * cópia de uma string existente
 */
char *
copy_string(char *s)
{
    if (s == NULL) {
        return NULL;
    } else {
        char *new = malloc_or_die(strlen(s) + 1);
        strcpy(new, s);

        return new;
    }
}

/* Variável `indentno` é usada pelo print_tree para
 * guardar o número atual da indentação
 */
static int indentno = 0;

/* Macros para aumentar/diminuir a indentação */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* Função `print_spaces` indenta printando espaços */
static void
print_spaces(void)
{
    for (int i = 0; i < indentno; i++) {
        fprintf(std_fd, " ");
    }
}

/* Função `print_types` printa os tipos de funções e variáveis */
static void
print_types(TreeNode *tree)
{
    if (tree->child[0] != NULL) {
        switch (tree->child[0]->type) {
        case Integer:
            fprintf(std_fd, "int");
            break;
        case Void:
            fprintf(std_fd, "void");
            break;
        case IntegerArray:
            fprintf(std_fd, "int[]");
            break;
        default: /* Não deve acontecer nunca */
            return;
        }
    } else {
        switch (tree->type) {
        case Integer:
            fprintf(std_fd, "int");
            break;
        case Void:
            fprintf(std_fd, "void");
            break;
        case IntegerArray:
            fprintf(std_fd, "int[]");
            break;
        default: /* Não deve acontecer nunca */
            return;
        }
    }
}

/* Função `print_tree` printa a árvore sintática para o
 * arquivo std_fd usando indentação para indicar sub-árvores
 */
void
print_tree(TreeNode *tree)
{
    INDENT;

    while (tree != NULL) {
        print_spaces();

        // IfK, WhileK, CompoundK, ReturnK
        if (tree->node_kind == StmtK) {
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(std_fd, "If\n");
                break;
            case WhileK:
                fprintf(std_fd, "While\n");
                break;
            case CompoundK:
                fprintf(std_fd, "Declaração composta\n");
                break;
            case ReturnK:
                fprintf(std_fd, "Return\n");
                break;
            case AssignK:
                fprintf(std_fd, "Atribuição:\n");
                break;
            default: /* Não deve acontecer nunca */
                fprintf(err_fd, "Tipo de declaração desconhecida\n");
                exit(EXIT_FAILURE);
            }
        }
        // OpK, ConstK, AssignK, IdK, TypeK, ArrIdK, CallK, CalcK
        else if (tree->node_kind == ExpK) {
            if (tree->kind.exp != TypeK)
                switch (tree->kind.exp) {
                case OpK:
                    fprintf(std_fd, "Op: ");
                    print_token(tree->attr.op, "\0");
                    break;
                case ConstK:
                    fprintf(std_fd, "Const: %d\n", tree->attr.val);
                    break;
                case IdK:
                    fprintf(std_fd, "Id: %s\n", tree->attr.name);
                    break;
                case TypeK:
                    /* Não faz nada */
                    break;
                case ArrIdK:
                    fprintf(std_fd, "ArrId: %s\n", tree->attr.name);
                    break;
                case CallK:
                    fprintf(std_fd, "Chamada de Função: %s\n", tree->attr.name);
                    break;
                case CalcK:
                    fprintf(std_fd, "Operador: ");
                    print_token(tree->child[1]->attr.op, "\0");
                    break;
                default: /* Não deve acontecer nunca */
                    fprintf(err_fd, "Tipo de expressão desconhecida\n");
                    exit(EXIT_FAILURE);
                }
        }
        // VarK, FunK, ArrVarK, ArrParamK, ParamK
        else if (tree->node_kind == DeclK) {
            switch (tree->kind.decl) {
            case FunK:
                fprintf(std_fd, "Declaração de Função:  ");
                print_types(tree);
                fprintf(std_fd, " %s()\n", tree->attr.name);
                break;
            case VarK:
                fprintf(std_fd, "Declaração de variável:  ");
                print_types(tree);
                fprintf(std_fd, " %s;\n", tree->attr.name);
                break;
            case ArrVarK:
                fprintf(std_fd, "Declaração de array de variável:  ");
                print_types(tree);
                fprintf(
                    std_fd, " %s[%d];\n", tree->attr.arr.name,
                    tree->attr.arr.size
                );
                break;
            case ArrParamK:
                fprintf(std_fd, "Parâmetro de array: %s\n", tree->attr.name);
                break;
            case ParamK:
                fprintf(std_fd, "Parâmetro: ");
                print_types(tree);
                if (tree->attr.name != NULL) {
                    fprintf(std_fd, " %s\n", tree->attr.name);
                } else {
                    fprintf(std_fd, " void\n");
                }
                break;
            default: /* Não deve acontecer nunca */
                fprintf(err_fd, "Declaração desconhecida\n");
                exit(EXIT_FAILURE);
            }
        } else { /* Não deve acontecer nunca */
            fprintf(err_fd, "Tipo de nó desconhecido\n");
            exit(EXIT_FAILURE);
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
CharStack
cs_init(void)
{
    int size = 256;
    CharStack new = malloc_or_die(sizeof(*new));

    new->max_size = size;
    new->last = -1; // Não há elementos
    new->items = malloc_or_die(size * sizeof(*new->items));

    return new;
}

/* Função `cs_push` coloca um elemento na pilha */
void
cs_push(CharStack stack, char *item)
{
    if (stack->last >= stack->max_size) { // Sem espaço, redimencionar
        stack->max_size *= 2;
        stack->items = realloc_or_die(
            stack->items, stack->max_size * sizeof(**stack->items)
        );
    }

    stack->last += 1;
    stack->items[stack->last] = malloc_or_die(strlen(item) + 1);

    strcpy(stack->items[stack->last], item);
}

/* Função `cs_pop` remove o elemento da pilha, retornando seu valor */
// NOTE: cabe a quem chamou a função limpar o item removido da memória
char *
cs_pop(CharStack stack)
{
    char *item = stack->items[stack->last];

    stack->items[stack->last] = NULL;
    stack->last -= 1;

    return item;
}

/* Função `cs_drop` remove a memória usada pela pilha */
void
cs_drop(CharStack stack)
{
    for (int i = 0; i <= stack->last; i++) {
        free(stack->items[i]);
    }

    free(stack->items);
    free(stack);
}

/* Função `cs_peek` retorna o elemento mais recente sem removê-lo da pilha */
char *
cs_peek(CharStack stack)
{
    if (stack->last < 0) { // Não há elementos
        return NULL;
    }

    return stack->items[stack->last];
}
