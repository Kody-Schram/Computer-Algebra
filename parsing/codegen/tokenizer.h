#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "../parserTypes.h"

/**
 * @brief Tokenizes a string input into an unrefined linked list of tokens
 * 
 * @retval NULL: Error tokenizing the string, space error, unrecognized character, or allocation erorr
 * @retval Token*: Properly tokenized the string
 * 
 * @param buffer Mathmatical string
 * @return Token* 
 */
Token *tokenize(char *buffer);

#endif