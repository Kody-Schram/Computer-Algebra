#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

const int DEFAULT_STACK_SIZE = 25;
const int DEFAULT_PARAM_SIZE = 3;

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
    else if (cur->type == TOKEN_IDENTIFIER && prev != NULL) {
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
 * @param prev Previous node in the list
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

/**
 * @brief Checks to ensure binary operators valid
 * 
 * @retval -1: Error, binary operator is invalid
 * @retval 0: All binary operators are valid
 * 
 * @param cur Current node in the list
 * @param prev Previous node in the list
 * @return int 
 */
int checkInvalidBinop(Token *cur, Token *prev) {
    Token *next = cur->next;
    if (prev != NULL && next != NULL) {
        if (cur->type == TOKEN_OPERATOR) {
            if (prev->type == TOKEN_OPERATOR || next->type == TOKEN_OPERATOR) {
                printf("Invalid operation \"%s%s%s\".\n", prev->value, cur->value, next->value);
                //printf("Two operators, \"%s%s\" are not allowed next to each other.\n", cur->value, next->value);
                return -1;

            } 
            else if (prev->type != TOKEN_NUMBER && prev->type != TOKEN_IDENTIFIER && prev->type != TOKEN_RIGHT_PAREN) {
                printf("Invalid operation \"%s%s%s\".\n", prev->value, cur->value, next->value);
                //printf("Operator must be preceeded by a number, an identifer, or a right parenthesis.\n");
                return -1;
            }
            else if (next->type != TOKEN_NUMBER && next->type != TOKEN_IDENTIFIER && next->type != TOKEN_LEFT_PAREN && next->type != TOKEN_FUNC_CALL) {
                printf("Invalid operation \"%s%s%s\".\n", prev->value, cur->value, next->value);
                //printf("Operator must be followed by a number, an identifier, a left parenthesis, or a function call.\n");
                return -1;
            }
        }
    }
    else if (cur->type == TOKEN_OPERATOR) {
        if (prev == NULL && next != NULL) {
            printf("Invalid operation \"%s%s\".\n", cur->value, next->value);
        } 
        else if (next == NULL && prev != NULL) {
            printf("Invalid operation \"%s%s\".\n", prev->value, cur->value);
        }
        else {
            printf("Invalid operation \"%s\"\n", cur->value);
        }

        return -1;
    }

    return 0;
}

int handleFunctionParens(Token **cur) {
    if ((*cur)->type == TOKEN_FUNC_CALL) {
        Token *func = *cur;
        if (func->next != NULL) {
            // Try to implicitly add opening parenthesis
            if (func->next->type != TOKEN_LEFT_PAREN) {
                Token *parenthesis = createToken(TOKEN_LEFT_PAREN, "(", 1);
                parenthesis->next = func->next;
                func->next = parenthesis;

                // Loops till end of term and adds closing parenthesis
                while (func->type != TOKEN_OPERATOR || func->value[0] == '*' || func->value[0] == '^')
                {
                    if (func->next == NULL) {
                        Token *closing = createToken(TOKEN_RIGHT_PAREN, ")", 1);
                        closing->next = NULL;
                        func->next = closing;

                        printf("closing parameter created \n");

                        return 0;
                    }

                    func = func->next;
                }

                Token *closing = createToken(TOKEN_RIGHT_PAREN, ")", 1);
                closing->next = func->next->next;
                func->next = closing;

                printf("closing parameter created \n");
            }
        }
        else {
            printf("Function call \"%s\" must be followed by required parameters.\n", (*cur)->value);
            return 1;
        }
    }

    return 0;
}

int parseFunctionParameters(Token *start) {
    if (start->type == TOKEN_FUNC_CALL) {
        // Create list of header tokens for parameters
        Token **parameters = malloc(DEFAULT_PARAM_SIZE * sizeof(Token*));
        int nParams = 0;

        Token *end = start->next->next;

        Token *cur = start->next->next;
        parameters[nParams] = cur;
        while (cur->next->type != TOKEN_SEPERATOR && cur->next->type != TOKEN_RIGHT_PAREN) {
            cur = cur->next;
        }
    }
    return 0;
}

void resizeStack(int *size, Token ***stack) {
    *size *= 2;
    Token **temp = realloc(*stack, *size * sizeof(Token*));

    if (temp == NULL) {
        printf("Error reallocating parenthesis stack.\n");
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
        // printf("Current Token: <%u, %s>\n", cur->type, cur->value);
        if (handleImplicitMul(cur, prev) == -1) return NULL;
        if (handleExponentRewrite(&cur, prev) == -1) return NULL;
        if (checkInvalidBinop(cur, prev) == -1) return NULL;
        if (handleFunctionParens(&cur)) return NULL;
        if (parseFunctionParameters(cur)) return NULL;

        // Counts open parenthesis
        if (cur->type == TOKEN_LEFT_PAREN) {
            openParenthesis ++;
        }

        if (cur->type == TOKEN_RIGHT_PAREN) {
            if (openParenthesis <= 0) {
                printf("Mismatched parenthesis.");
                return NULL;
            }

            openParenthesis --;
        }

        prev = cur;
        cur = cur->next;
    }

    return head;

}

// 2(x+1