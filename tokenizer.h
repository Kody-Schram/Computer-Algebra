#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER
} TokenType;

typedef struct Token {
    TokenType type;
    char* value;
    struct Token* next;
} Token;

Token *createToken(TokenType type, char *value, int l);

void printTokens(Token *head);

int getOperatorLength(char *c);

Token *tokenize(char *expression);

void freeTokens(Token *head);

#endif