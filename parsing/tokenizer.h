#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "parserTypes.h"

Token *createToken(TokenType type, char *value, int l);

Token *tokenize(char *expression);

#endif