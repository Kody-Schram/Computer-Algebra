#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <stdio.h>

#include "core/primitives/types.h"
#include "parser_types.h"


Token *createTokenOperator(const Operation *op);

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
Token *createToken(TokenType type, const char *value, int l);

/**
 * @brief Prints linked token list in a easy to read format
 * 
 * @param head Head of linked list
 */
FILE *printTokens(const Token *head);


Expression *createExpression(const Token *token);


FILE *printRPN(const RPNList *list);

/**
 * @brief Frees the memory of old tokens
 * 
 * @param head Head of linked list
 */
void freeTokens(Token *head);


#endif
