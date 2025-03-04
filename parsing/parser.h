#ifndef PARSER_H
#define PARSER_H

#include "parserTypes.h"

/**
 * @brief Frees the memory of old tokens
 * 
 * @param head Head of linked list
 */
void freeTokens(Token *head);

/**
 * @brief Prints linked token list in a easy to read format
 * 
 * @param head Head of linked list
 */
void printTokens(Token *head);

/**
 * @brief Handles entire parsing process from string to AST
 * 
 * @param buffer Mathematical string
 * @return Token* 
 */
Token *parse(char *buffer);

#endif