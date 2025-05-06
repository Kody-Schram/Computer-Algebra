#ifndef AST_H
#define AST_H

#include "../parserTypes.h"

typedef struct RPNList {
    int length;
    Token **rpn;
} RPNList;

typedef struct Stack {
    int length;
    int entries;
    union {
        Token **tokens;
        ASTNode **nodes;
    };
} Stack;

RPNList shuntingYard(Token *head);

#endif