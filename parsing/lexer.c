#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "parserTypes.h"
#include "tokenizer.h"

int handleImplicitMul(Token *cur, Token *prev) {
    // Checks for implicit multiplcation before left bracket
    if (cur->type == TOKEN_LEFT_PAREN && prev != NULL) {
        if (prev->type == TOKEN_IDENTIFIER || prev->type == TOKEN_NUMBER || prev->type == TOKEN_RIGHT_PAREN) {
            Token *impMult = createToken(TOKEN_OPERATOR, "*", 1);
            if (impMult == NULL) {
                printf("Error allocating for implicit multiplcation token.\n");
                return 0;
            }

            prev->next = impMult;
            impMult->next = cur;

            return 1;

        }
    }

    // Checks for implicit multiplcation after right bracket
    else if (cur->type == TOKEN_RIGHT_PAREN && cur->next != NULL) {
        Token *nextToken = cur->next;
        if (nextToken->type == TOKEN_IDENTIFIER || nextToken->type == TOKEN_NUMBER || nextToken->type == TOKEN_LEFT_PAREN) {
            Token *impMult = createToken(TOKEN_OPERATOR, "*", 1);
            if (impMult == NULL) {
                printf("Error allocating for implicit multiplcation token.\n");
                return 0;
            }

            cur->next = impMult;
            impMult->next = nextToken;

            return 1;

        }
    }

    // Checks number to variable implicit multiplication (eg. 2x)
    else if (cur->type == TOKEN_NUMBER && cur->next != NULL) {
        if (cur->next->type == TOKEN_IDENTIFIER) {
            Token *impMult = createToken(TOKEN_OPERATOR, "*", 1);
            if (impMult == NULL) {
                printf("Error allocating for implicit multiplcation token.\n");
                return 0;
            }

            impMult->next = cur->next;
            cur->next = impMult;
        }
    }

    return 0;
}

int handleExponentRewrite(Token **cur, Token *prev) {
    if ((*cur)->value[0] == '*' && (*cur)->next != NULL) {
        if ((*cur)->next->value[0] == '*') {
            Token *exponent = createToken(TOKEN_OPERATOR, "^", 1);
            if (exponent == NULL) {
                printf("Error allocating for ** to ^ conversion token.");
                return 0;
            }

            prev->next = exponent;
            exponent->next = (*cur)->next->next;

            free((*cur)->next->value);
            free((*cur)->next);

            *cur = exponent;

            return 1;
        }
    }

    return 0;
}

// Cleans the token list
Token *lex(Token* head) {
    Token *cur = head;
    Token *prev = NULL;

    while (cur != NULL) {
        if (handleImplicitMul(cur, prev)) continue;
        else if (handleExponentRewrite(&cur, prev)) continue;

        prev = cur;
        cur = cur->next;
    }

    return head;

}