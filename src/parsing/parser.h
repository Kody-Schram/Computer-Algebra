#ifndef PARSER_H
#define PARSER_H

#include "utils/types.h"
#include "parserTypes.h"

/**
 * @brief Frees the memory of old tokens
 * 
 * @param head Head of linked list
 */
void freeTokens(Token *head);


int parseFunction(Token *head);

/**
 * @brief Handles entire parsing process from string to AST
 * 
 * @retval NULL: Error parsing the string
 * @retval Token*: Properly parsed the string
 * 
 * @param buffer Mathematical string
 * @return Token* 
 */
ASTNode *parse(char *buffer, Environment *env, int debugging);

#endif