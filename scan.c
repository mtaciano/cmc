/* Implementação do scanner da linguagem C- */

#include "globals.h"
#include "scan.h"
#include "util.h"

/* estados no DFA do scanner */
typedef enum {
    START,
    INNUM,
    INID,
    INLT,
    INGT,
    INNE,
    INASSIGN,
    INOVER,
    INCOMMENT,
    EXITINGCOMMENT,
    DONE
} StateType;

/* lexema do identificador ou palavra reservada */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = tamaho do  input buffer para
   as linhas do código */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* linha atual */
static int linepos = 0;      /* posição atual no LineBuf */
static int bufsize = 0;      /* tamanho atual do buffer string */
static int EOF_flag =
    FALSE; /* arruma o comportamento do ungetNextChar no EOF */

/* getNextChar retorna o próximo caractere não branco
   no lineBuf, lendo uma nova linha de lineBuf se a linha
   foi totalmente consumida */
static int getNextChar(void) {
    if (!(linepos < bufsize)) {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source)) {
            if (EchoSource)
                fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        } else {
            EOF_flag = TRUE;
            return EOF;
        }
    } else
        return lineBuf[linepos++];
}

/* ungetNextChar volta um caractere
   no lineBuf */
static void ungetNextChar(void) {
    if (!EOF_flag)
        linepos--;
}

/* tabela de consultas das palavras reservadas */
static struct {
    char *str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {{"else", ELSE}, {"if", IF},
                                {"int", INT},   {"return", RETURN},
                                {"void", VOID}, {"while", WHILE}};

/* Consulta um identificador para ver se é uma palavra reservada */
/* usando busca linear */
static TokenType reservedLookup(char *s) {
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/* Função getToken retorna o
 * próximo token do arquivo fonte
 */
TokenType getToken(void) { /* Index para guardar para o tokenString */
    int tokenStringIndex = 0;
    /* Guarda o token atual para ser retornado */
    TokenType currentToken;
    /* Estado atual - sempre começa em START */
    StateType state = START;
    /* Flag para indicar salvar para o tokenString */
    int save;
    while (state != DONE) {

        int c = getNextChar();
        save = TRUE;

        switch (state) {
        case START:

            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == '<')
                state = INLT;
            else if (c == '>')
                state = INGT;
            else if (c == '!')
                state = INNE;
            else if (c == '=')
                state = INASSIGN;
            else if (c == '/')
                state = INOVER;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else {
                state = DONE;
                switch (c) {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case '[':
                    currentToken = LBRACKET;
                    break;
                case ']':
                    currentToken = RBRACKET;
                    break;
                case '{':
                    currentToken = LBRACE;
                    break;
                case '}':
                    currentToken = RBRACE;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                case ',':
                    currentToken = COMMA;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;

        case INNUM:
            if (!isdigit(c)) { /* Volta o input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;

        case INID:

            if (!isalpha(c)) { /* Volta o input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;

        case INLT: /* <, <= */

            state = DONE;
            if (c == '=') {
                currentToken = LE;
            } else {
                /* Volta o input */
                ungetNextChar();
                save = FALSE;
                currentToken = LT;
            }
            break;

        case INGT: /* >, >= */

            state = DONE;
            if (c == '=') {
                currentToken = GE;
            } else {
                /* Volta o input */
                ungetNextChar();
                save = FALSE;
                currentToken = GT;
            }
            break;

        case INNE: /* !=, apenas "!" não existe */

            state = DONE;
            if (c == '=') {
                currentToken = NE;
            } else {
                /* Volta o input */
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;

        case INASSIGN:

            state = DONE;
            if (c == '=') {
                currentToken = EQ;
            } else {
                /* Volta o input */
                ungetNextChar();
                save = FALSE;
                currentToken = ASSIGN;
            }
            break;

        case INOVER:
            if (c == '*') {
                save = FALSE;
                state = INCOMMENT;
            } else {
                /* Volta o input */
                state = DONE;
                ungetNextChar();
                save = FALSE;
                currentToken = OVER;
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF) {
                state = DONE;
                currentToken = ENDFILE;
            } else if (c == '*') {
                state = EXITINGCOMMENT;
            }
            break;
        case EXITINGCOMMENT:
            save = FALSE;
            if (c == EOF) {
                state = DONE;
                currentToken = ENDFILE;
            } else if (c == '/') {
                state = START;
            } else {
                state = INCOMMENT;
            }
            break;
        case DONE: /* Não faz nada */
        default:   /* Nunca deve acontecer */
            fprintf(listing, "Bug no scanner: estado = %d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }

        if ((save) && (tokenStringIndex <= MAXTOKENLEN)) {
            tokenString[tokenStringIndex++] = (char)c;
        }

        if (state == DONE) {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }

    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        print_token(currentToken, tokenString);
    }
    return currentToken;
} /* fim getToken */
