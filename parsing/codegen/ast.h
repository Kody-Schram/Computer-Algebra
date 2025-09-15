#ifndef AST_H
#define AST_H

#include "../parserTypes.h"

typedef struct RPNList {
    int length;
    Token **rpn;
} RPNList;

typedef struct Stack {
    int size;
    int entries;
    void **items;
} Stack;

RPNList *shuntingYard(Token *head);

#endif