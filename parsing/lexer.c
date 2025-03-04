#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tokenizer.h"

// Cleans the token list
Token *lex(Token* head) {
    Token *cur = head;
    Token *prev = NULL;

    while (cur != NULL) {
        // Handles implicit multiplcation
        if (cur->type == TOKEN_LEFT_BRACKET && prev != NULL) {
            if (prev->type == TOKEN_IDENTIFIER || prev->type == TOKEN_NUMBER || prev->type == TOKEN_RIGHT_BRACKET) {
                Token *impMult = createToken(TOKEN_OPERATOR, "*", 1);
                if (impMult == NULL) {
                    printf("Error allocating for implicit multiplcation token.");
                    return NULL;
                }

                prev->next = impMult;
                impMult->next = cur;

            }
        }

        prev = cur;
        cur = cur->next;
    }

    return head;

}