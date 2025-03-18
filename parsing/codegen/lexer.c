#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"

const int DEFAULT_STACK_SIZE = 25;

/**
 * @brief Adds * where it is implied through standard math notation
 * 
 * @retval -1: Error, could not complete the function
 * @retval 0: No implicit multiplcation found, no action taken
 * @retval 1: Properly identified and corrected implied multiplcation
 * 
 * @param cur Current token in list
 * @param prev Previous token in list
 * @return int 
 */
int handleImplicitMul(Token *cur, Token *prev) {
    //printf("checking for implicit\n");
    // x() or 2()
    // 2x
    // xsinx

    // Handle implicit multiplcation with left bracket or function
    // x(x-1) => x*(x-1)
    if ((cur->type == TOKEN_LEFT_PAREN || cur->type == TOKEN_FUNC_CALL) && prev != NULL) {
        //printf("checking for left paren and function\n");
        if (prev->type == TOKEN_NUMBER || prev->type == TOKEN_IDENTIFIER) {
            Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
            if (mult == NULL) {
                printf("Error allocating for implicit multiplcation token.");
                return -1;
            }

            prev->next = mult;
            mult->next = cur;

            return 1;
        }
    }
    // Handle implicit multiplcation with right bracket
    // (x-1)x => (x-1)*x
    else if (cur->type == TOKEN_RIGHT_PAREN && cur->next != NULL) {
        //printf("checking for right paren\n");
        Token *nextToken = cur->next;
        if (nextToken->type == TOKEN_NUMBER || nextToken->type == TOKEN_IDENTIFIER || nextToken->type == TOKEN_LEFT_PAREN || nextToken->type == TOKEN_FUNC_CALL) {
            Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
            if (mult == NULL) {
                printf("Error allocating for implicit multiplcation token.");
                return -1;
            }

            mult->next = nextToken;
            cur->next = mult;

            return 1;
        }
    }
    // Handle constant and identifier multiplication
    // 2x => 2*x
    else if (cur->type == TOKEN_NUMBER && prev != NULL) {
        if (prev->type == TOKEN_NUMBER) {

            Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
            if (mult == NULL) {
                printf("Error allocating for implicit multiplcation token.");
                return -1;
            }

            prev->next = mult;
            mult->next = cur;

            return 1;
        }
    }

    return 0;
}

/**
 * @brief Replaces ** tokens with a ^ token
 * 
 * @retval -1: Error, could not properly create the new token
 * @retval 0: "**" tokens not found, no replacement made
 * @retval 1: Properly replaced with "^"
 * 
 * @param cur Current node in the list
 * @param prev Previous node
 * @return int 
 */
int handleExponentRewrite(Token **cur, Token *prev) {
    if ((*cur)->value[0] == '*' && (*cur)->next != NULL) {
        if ((*cur)->next->value[0] == '*') {
            Token *exponent = createToken(TOKEN_OPERATOR, "^", 1);
            if (exponent == NULL) {
                printf("Error allocating for '**' to '^' conversion token.");
                return -1;
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

int checkInvalidBinop(Token *cur, Token *prev) {
    if (prev != NULL && cur->next != NULL) {
        if (cur->type == TOKEN_OPERATOR && (prev->type == TOKEN_OPERATOR || cur->next->type == TOKEN_OPERATOR)) {
            printf("Two operators are not allowed next to each other.\n");
            return 1;
        }
    }
    else if (cur->type == TOKEN_OPERATOR) {
        printf("Invalid binary operator.\n");
        if (prev == NULL) {
            printf("Operator must be preceeded by a variable, number, or function.\n");
        } 
        if (cur->next == NULL) {
            printf("Operator must be followed by a variable, number, or function.\n");
        }

        return 1;
    }

    return 0;
}

void resizeStack(int *size, Token ***stack) {
    *size *= 2;
    Token **temp = realloc(*stack, *size * sizeof(Token*));

    if (temp == NULL) {
        printf("Error reallocating parenthesis stack.");
        free(*stack);
        *stack = NULL;
    } else {
        *stack = temp;
    }
}

Token *lex(Token* head) {
    Token *cur = head;
    Token *prev = NULL;

    int openParenthesis = 0;
    int size = DEFAULT_STACK_SIZE;
    Token **stack = malloc(size * sizeof(Token*));
    if (stack == NULL) {
        printf("Error allocating for parenthesis stack.\n");
        return NULL;
    }

    while (cur != NULL) {
        printf("Current Token: <%u, %s>\n", cur->type, cur->value);
        if (handleImplicitMul(cur, prev) == -1) return NULL;
        if (handleExponentRewrite(&cur, prev) == -1) return NULL;
        if (checkInvalidBinop(cur, prev)) return NULL;

        if (cur->type == TOKEN_LEFT_PAREN) {
            openParenthesis ++;

            if (openParenthesis > size) {
                resizeStack(&size, &stack);
                if (stack == NULL) return NULL;
            }

            stack[openParenthesis] = cur;
        }

        prev = cur;
        cur = cur->next;
    }

    return head;

}