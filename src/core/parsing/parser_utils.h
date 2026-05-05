#pragma once

#include <stdio.h>

#include "core/primitives/types.h"
#include "parser_types.h"


Token *createOperatorToken(Operation const *op);


Token *createFuncCallToken(Component const *cmp);

/**
 * @brief Creates a Token object
 * 
 * @retval NULL: Error creating the new token
 * @retval Token*: Properly created the new token
 * 
 * @param type The type of the Token
 * @param value String value of the Token
 * @param l Length of the string value
 * @return Token* 
 */
Token *createToken(TokenType type, char const *value, int l);

/**
 * @brief Prints linked token list in a easy to read format
 * 
 * @param head Head of linked list
 */
FILE *printTokens(Token const *head);


Expression *createExpression(Token const *token);


FILE *printRPN(RPNList const *list);

/**
 * @brief Frees the memory of old tokens
 * 
 * @param head Head of linked list
 */
void freeTokens(Token *head);
