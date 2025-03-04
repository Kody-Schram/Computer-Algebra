#ifndef LEXER_H
#define LEXER_H

#include "parserTypes.h"

/**
 * @brief Refines tokens list for AST generation
 * 
 * @param head Head of linked list
 * @return Token* 
 */
Token *lex(Token *head);

#endif