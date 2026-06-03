#pragma once

#include <stdint.h>
#include "core/common.h"


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

    union {
        char *value;
		Expression *finalizedCall;
		const Component *cmp;
        const Operation *op;
    };

    struct Token *next;
};


// RPN related definitions
struct RPNList {
    uint32_t length;
    Token **items;
};

struct Stack {
    uint32_t size;
    uint32_t entries;
    void **items;
};
