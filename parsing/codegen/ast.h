#ifndef AST_H
#define AST_H

#include "../parserTypes.h"

typedef struct RPNList {
    int length;
    Token **rpn;
} RPNList;

RPNList shuntingYard(Token *head);

#endif