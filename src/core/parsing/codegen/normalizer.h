#ifndef NORMALIZER_H 
#define NORMALIZER_H

#include "core/parsing/parser_types.h"


int handleLocalVariables(Token **ptr, char **parameters, int nParameters);

/**
 * @brief Refines tokens list for AST generation
 * 
 * @retval NULL: Error lexing the token list
 * @retval Token*: Properly lexed the list, return head of list
 * 
 * @param head Head of linked list
 * @return Token* 
 */
int normalize(Token **head);

#endif
