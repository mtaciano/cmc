/* Funções do scanner da linguagem C- */

#ifndef _SCAN_H_
#define _SCAN_H_

#include "common/globals.h"

/* Constante `MAX_TOKEN_LEN` é o tamanho máximo de um token */
#define MAX_TOKEN_LEN 40

/* Variável `token_string` armazena o lexema de cada token */
extern char token_string[MAX_TOKEN_LEN + 1];

/* Função `get_token` retorna o token atual */
TokenType get_token(void);

#endif /* _SCAN_H_ */
