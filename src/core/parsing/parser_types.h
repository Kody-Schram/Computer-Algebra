#ifndef PARSERTYPES_H
#define PARSERTYPES_H

#include "core/primitives/types.h"

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
    TOKEN_FUNC_CALL_PLACEHOLDER,
    TOKEN_FUNC_CALL,
    TOKEN_MAPPING,
    TOKEN_SEPARATOR
};

struct Token {
    TokenType type;
	unsigned int nParams;

    union {
        char *value;
        FunctionCall *call;
        const Operation *op;
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
