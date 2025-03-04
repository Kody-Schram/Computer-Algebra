#ifndef PARSER_H
#define PARSER_H

#include "parserTypes.h"

void freeTokens(Token *head);

void printTokens(Token *head);

Token *parse(char *buffer);

#endif