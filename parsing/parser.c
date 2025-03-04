#include <stdlib.h>
#include <stdio.h>

#include "parserTypes.h"
#include "tokenizer.h"
#include "lexer.h"

void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
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
            case TOKEN_FUNCTION:
                type = "FUNCTION";
                break;
            case TOKEN_LEFT_PAREN:
                type = "LEFT_PAREN";
                break;
            case TOKEN_RIGHT_PAREN:
                type = "RIGHT_PAREN";
                break;
            case TOKEN_SEPERATOR:
                type = "SEPERATOR";
                break;
            case TOKEN_FUNC_DEF:
                type = "FUNC_DEF";
                break;
        }

        printf("    <type: %s, value: '%s'>\n", type, cur->value);
        cur = cur->next;
    }

    printf("]\n");
}

Token *parse(char *buffer) {
    Token *raw = tokenize(buffer);

    printf("\nRaw Tokens\n");
    printTokens(raw);

    Token *head = lex(raw);

    return head;
}