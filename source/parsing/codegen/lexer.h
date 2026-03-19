#ifndef LEXER_H
#define LEXER_H

#include "../parserTypes.h"


void handleLocalVariables(Token **ptr, Environment *env, int nParams, char **parameters);

/**
 * @brief Refines tokens list for AST generation
 * 
 * @retval NULL: Error lexing the token list
 * @retval Token*: Properly lexed the list, return head of list
 * 
 * @param head Head of linked list
 * @return Token* 
 */
Token *lex(Token *head);

#endif