#ifndef LEXER_H
#define LEXER_H

#include "utils/types.h"
#include "parsing/parserTypes.h"


int handleLocalVariables(Token **ptr, Environment *localEnv);

/**
 * @brief Refines tokens list for AST generation
 * 
 * @retval NULL: Error lexing the token list
 * @retval Token*: Properly lexed the list, return head of list
 * 
 * @param head Head of linked list
 * @return Token* 
 */
int lex(Token **head);

#endif