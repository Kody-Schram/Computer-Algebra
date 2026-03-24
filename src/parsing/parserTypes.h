#ifndef PARSERTYPES_H
#define PARSERTYPES_H

#include "utils/types.h"

// Forward declaring types
typedef enum TokenType TokenType;
typedef struct Token Token;

typedef struct RPNList RPNList;
typedef struct Stack Stack;


// Token related definitions
enum TokenType {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGNMENT,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_FUNC_CALL,
    TOKEN_FUNC_DEF,
    TOKEN_SEPARATOR
};

struct Token {
    TokenType type;

    union {
        char *value;
        FunctionCall *call;
    };

    struct Token *next;
};


// RPN related definitions
struct RPNList {
    int length;
    Token **items;
};

struct Stack {
    int size;
    int entries;
    void **items;
};

#endif