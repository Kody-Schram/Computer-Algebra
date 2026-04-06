#ifndef PARSER_H
#define PARSER_H

#include "utils/types.h"
#include "parserTypes.h"


int parseFunction(Token *head);

/**
 * @brief Handles entire parsing process from string to AST
 * 
 * @retval nullptr: Error parsing the string
 * @retval Token*: Properly parsed the string
 * 
 * @param buffer Mathematical string
 * @return Token* 
 */
ASTNode *parse(char *buffer);

#endif