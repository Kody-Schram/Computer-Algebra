#ifndef PARSERTYPES_H
#define PARSERTYPES_H

#include "tables/functions.h"

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
    union {
        char *identifier;
        double value;
        Function *function;
    };

    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/**
 * @brief Creates a Token object
 * 
 * @retval NULL: Error creating the new token
 * @retval Token*: Properly created the new token
 * 
 * @param type The type of the Token
 * @param value String value of the Token
 * @param l Length of the string value
 * @return Token* 
 */
Token *createToken(TokenType type, char *value, int l);


void printTokens(Token *head);


ASTNode *createASTNode(Token *token);

#endif