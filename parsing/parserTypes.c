#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parserTypes.h"

Token *createToken(TokenType type, char *value, int l) {
    // Allocates new token
    Token *token = (Token*) malloc(sizeof(Token));
    if (token == NULL) {
        printf("Error allocating space for token.");
        return NULL;
    }

    // Populates newToken attributes
    token->type = type;
    token->value = (char*) malloc(l+1);
    if (token->value == NULL) {
        printf("Error allocating space for token value.");
        return NULL;
    }
    memcpy(token->value, value, l);
    token->value[l] = '\0';
    token->next = NULL;

    return token;
}

void printTokens(Token *head) {
    Token *cur = head;

    printf("[\n");
    while (cur != NULL) {
        const char *type = NULL;

        switch(cur->type) {
            case TOKEN_NUMBER:
                type = "NUMBER";
                break;
            case TOKEN_OPERATOR:
                type = "OPERATOR";
                break;
            case TOKEN_IDENTIFIER:
                type = "IDENTIFIER";
                break;
            case TOKEN_ASSIGNMENT:
                type = "ASSIGNMENT";
                break;
            case TOKEN_LEFT_PAREN:
                type = "LEFT_PAREN";
                break;
            case TOKEN_RIGHT_PAREN:
                type = "RIGHT_PAREN";
                break;
            case TOKEN_FUNC_CALL:
                type = "FUNC_CALL";
                break;
            case TOKEN_FUNC_DEF:
                type = "FUNC_DEF";
                break;
            case TOKEN_SEPERATOR:
                type = "SEPERATOR";
                break;
        }

        printf("    <type: %s, value: '%s'>\n", type, cur->value);
        cur = cur->next;
    }

    printf("]\n");
}