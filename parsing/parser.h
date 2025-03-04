#ifndef PARSER_H
#define PARSER_H

#include "parserTypes.h"

Token *parse(char *buffer);

void freeTokens(Token *head);

void printTokens(Token *head);

#endif