#include <stdlib.h>
#include <stdio.h>

#include "parserTypes.h"
#include "tokenizer.h"
#include "lexer.h"

Token *parse(char *buffer) {
    Token *raw = tokenize(buffer);
    Token *head = lex(raw);

    return head;
}

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
            case TOKEN_LEFT_BRACKET:
                type = "LEFT_BRACKET";
                break;
            case TOKEN_RIGHT_BRACKET:
                type = "RIGHT_BRACKET";
                break;
            case TOKEN_SEPERATOR:
                type = "SEPERATOR";
                break;
            case TOKEN_FUNC_DEF:
                type = "FUNC_DEF";
                break;
        }

        printf("<type: %s, value: %s>\n", type, cur->value);
        cur = cur->next;
    }
}