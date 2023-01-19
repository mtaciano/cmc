/* Especificação C- Lex/Flex */
%option noyywrap
%option nounput
%option never-interactive

%{
#include "../src/common/globals.h"
#include "../src/common/util.h"
#include "../src/scan.h"

char token_string[MAX_TOKEN_LEN + 1];

extern int yylex(void);
%}

digit        [0-9]
NUM          {digit}+
letter       [a-zA-Z_]
ID           {letter}+
newline      [\n\r]+
whitespace   [ \t]+

%%

"if"            { return IF; }
"int"           { return INT; }
"else"          { return ELSE; }
"return"        { return RETURN; }
"void"          { return VOID; }
"while"         { return WHILE; }
"=="            { return EQ; }
"="             { return ASSIGN; }
"<"             { return LT; }
"<="            { return LE; }
">="            { return GE; }
">"             { return GT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return OVER; }
"!="            { return NE; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMI; }
","             { return COMMA; }
{NUM}           { return NUM; }
{ID}            { return ID; }
{newline}       { lineno++; }
{whitespace}    { /* continue; */ }
"/*"            {
                    char c = '\0';
                    char previous_c = '\0';

                    while (1) {
                        c = input();
                        if (c == EOF) break;
                        if (c == '\n') lineno++;
                        if (previous_c == '*' && c == '/') break;
                        previous_c = c;
                    }
                }
.               { return ERROR; }

%%

/* Função `get_token` retorna o token atual */
TokenType get_token(void) {
    static int first_time = true;
    TokenType current_token;

    if (first_time) {
        first_time = false;
        lineno++;
        yyin = source;
        yyout = std_fd;
    }

    current_token = yylex();
    strncpy(token_string, yytext, MAX_TOKEN_LEN);

    if (g_trace_scan) {
        fprintf(std_fd, "\t%d: ", lineno);
        print_token(current_token, token_string);
    }

    return current_token;
}
