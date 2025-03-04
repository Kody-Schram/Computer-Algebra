#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_FUNCTION,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEPERATOR,
    TOKEN_FUNC_DEF
} TokenType;

typedef struct Token {
    TokenType type;
    char* value;
    struct Token* next;
} Token;

Token *createToken(TokenType type, char *value, int l);

void printTokens(Token *head);

Token *tokenize(char *expression);

void freeTokens(Token *head);

#endif