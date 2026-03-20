#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "parsing/parserTypes.h"
#include "utils/types.h"

/**
 * @brief Tokenizes a string input into an unrefined linked list of tokens
 * 
 * @retval NULL: Error tokenizing the string
 * @retval Token*: Head of Token linked list
 * 
 * @param buffer Mathmatical string
 * @return Token* 
 */
Token *tokenize(char *buffer, Environment *env);

#endif