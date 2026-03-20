#ifndef PARSER_H
#define PARSER_H

#include "utils/types.h"
#include "utils/config.h"
#include "parserTypes.h"




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
ASTNode *parse(char *buffer, Environment *env, Config *config);

#endif