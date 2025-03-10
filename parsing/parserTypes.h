#ifndef PARSERTYPES_H
#define PARSERTYPES_H

typedef enum TokenType {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGNMENT,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_FUNC_CALL,
    TOKEN_FUNC_DEF,
    TOKEN_SEPERATOR
} TokenType;

typedef struct Token {
    TokenType type;
    char* value;

    struct Token* next;
} Token;


typedef enum NodeType {
    NODE_NUMBER,
    NODE_OPERATOR,
    NODE_VARIABLE,
    NODE_FUNC_CALL,
    NODE_FUNC_DEF
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char *value;

    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

#endif