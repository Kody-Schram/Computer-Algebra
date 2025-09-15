#ifndef AST_H
#define AST_H

#include "../parserTypes.h"

typedef struct RPNList {
    int length;
    Token **items;
} RPNList;

typedef struct Stack {
    int size;
    int entries;
    void **items;
} Stack;

RPNList *shuntingYard(Token *head);


ASTNode *astFromRPN(RPNList *rpn);

#endif