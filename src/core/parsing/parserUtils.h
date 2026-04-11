#ifndef PARSERUTILS_H
#define PARSERUTILS_H

#include <stdio.h>

#include "core/utils/types.h"
#include "parserTypes.h"

/**
 * @brief Creates a Token object
 * 
 * @retval nullptr: Error creating the new token
 * @retval Token*: Properly created the new token
 * 
 * @param type The type of the Token
 * @param value String value of the Token
 * @param l Length of the string value
 * @return Token* 
 */
Token *createToken(TokenType type, char *value, int l);

/**
 * @brief Prints linked token list in a easy to read format
 * 
 * @param head Head of linked list
 */
FILE *printTokens(Token *head);


Expression *createExpression(Token *token);


FILE *printRPN(RPNList *list);

/**
 * @brief Frees the memory of old tokens
 * 
 * @param head Head of linked list
 */
void freeTokens(Token *head);


#endif