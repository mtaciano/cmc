/* Especificação C- Lex/Flex */
%option noyywrap
%option nounput

%{
#include "globals.h"
#include "util.h"
#include "scan.h"

char tokenString[MAXTOKENLEN+1];
extern int yylex(void);
%}

digit       [0-9]
NUM          {digit}+
letter        [a-zA-Z]
ID           {letter}+
newline      \n
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
                  char c;
                  while (1) {
                    c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                    while ((c = input()) == '*');
                    if (c == '/') break;
                  }
                }
.               { return ERROR; }

%%

/* Função getToken retorna o token atual */
TokenType getToken(void) {
  static int firstTime = TRUE;
  TokenType currentToken;

  if (firstTime) {
    firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }

  currentToken = yylex();
  strncpy(tokenString, yytext, MAXTOKENLEN);

  if (TraceScan) {
    fprintf(listing, "\t%d: ", lineno);
    print_token(currentToken, tokenString);
  }

  return currentToken;
}
