#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "parserTypes.h"

/**
 * @brief Creates a Token object
 * 
 * @param type The type of the Token
 * @param value String value of the Token
 * @param l Length of the string value
 * @return Token* 
 */
Token *createToken(TokenType type, char *value, int l);

/**
 * @brief Tokenizes a string input into an unrefined linked list of tokens
 * 
 * @param buffer Mathmatical string
 * @return Token* 
 */
Token *tokenize(char *buffer);

#endif