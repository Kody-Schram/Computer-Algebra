#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "lexer.h"
#include "utils/context/context.h"
#include "utils/log.h"
#include "parsing/parserUtils.h"

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
static int handleImplicitMul(Token *cur, Token *prev) {
    // Handle implicit multiplcation with left bracket or function
    // x(x-1) => x*(x-1)
    if ((cur->type == TOKEN_LEFT_PAREN || cur->type == TOKEN_FUNC_CALL_PLACEHOLDER) && prev != NULL) {
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
        if (nextToken->type == TOKEN_NUMBER || nextToken->type == TOKEN_IDENTIFIER || nextToken->type == TOKEN_LEFT_PAREN || nextToken->type == TOKEN_FUNC_CALL_PLACEHOLDER) {
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
static int handleExponentRewrite(Token **cur, Token *prev) {
    if ((*cur)->value[0] != '*' || !((*cur)->next == NULL && (*cur)->next->value[0] != '*')) return 0;
    Debug(0, "Rewriting exponent\n");

    Token *exponent = createToken(TOKEN_OPERATOR, "^", 1);
    if (exponent == NULL) {
        printf("Error allocating for '**' to '^' conversion token.");
        return -1;
    }

    exponent->next = (*cur)->next->next;

    free((*cur)->next->value);
    free((*cur)->next);
    free((*cur)->value);
    free(*cur);

    if (prev != NULL) prev->next = exponent;
    else *cur = exponent;

    return 1;
}

/**
 * @brief Checks to ensure binary operators valid
 * 
 * @retval -1: Invalid binary operation
 * @retval 0: No binary operation
 * @retval 1: Binary operator is valid
 * 
 * @param cur Current node in the list
 * @param prev Previous node in the list
 * @return int Error code
 */
static int checkInvalidBinop(Token *cur, Token *prev) {
    if (cur->type != TOKEN_OPERATOR) return 0;
    if (cur->next == NULL) {
        printf("Operator must be followed by another token.\n");
        return -1;
    }

    if (prev == NULL) {
        printf("Operator must be preceeded by another token.\n");
        return -1;
    }

    // If next op is -, will recheck later, may be a negative number instead of a minus
    if (cur->next->type == TOKEN_OPERATOR && cur->next->value[0] == '-') return 0;

    // Handles invalid negative signs
    if (cur->value[0] == '-' && (cur->next == NULL || cur->next->type == TOKEN_OPERATOR)) {
        printf("Invalid operation \"%s %s %s\".\n", prev->value, cur->value, cur->next->value);
        return -1;
    }

    if (cur->next->type != TOKEN_IDENTIFIER && cur->next->type != TOKEN_NUMBER && cur->next->type != TOKEN_FUNC_CALL_PLACEHOLDER && cur->next->type != TOKEN_LEFT_PAREN) {
        printf("Invalid operation \"%s %s %s\".\n", prev->value, cur->value, cur->next->value);
        return -1;
    }
    
    return 1;
}

/**
 * @brief Adds parenthesis around first term after function call if none are found
 * 
 * @retval -1: Error
 * @retval 0: No parenthesis added
 * @retval 1: Parenthesis added
 * 
 * @param cur Current node in the list
 * @return int Error code
 */
static int handleFunctionParens(Token **cur) {
    // revisit
    if ((*cur)->type == TOKEN_FUNC_CALL_PLACEHOLDER) {
        Token *func = *cur;
        if (func->next != NULL) {
            // Try to implicitly add opening parenthesis
            if (func->next->type != TOKEN_LEFT_PAREN) {
                Token *parenthesis = createToken(TOKEN_LEFT_PAREN, "(", 1);
                if (parenthesis == NULL) {
                    printf("Error creating the opening parenthesis token.\n");
                    return -1;
                }
                parenthesis->next = func->next;
                func->next = parenthesis;

                // Loops till end of term and adds closing parenthesis
                while (func->type != TOKEN_OPERATOR || func->value[0] == '*' || func->value[0] == '^')
                {
                    if (func->next == NULL) {
                        Token *closing = createToken(TOKEN_RIGHT_PAREN, ")", 1);
                        if (closing == NULL) {
                            printf("Error creating the closing parenthesis token.\n");
                            return -1;
                        }
                        closing->next = NULL;
                        func->next = closing;

                        return 1;
                    }

                    func = func->next;
                }

                Token *closing = createToken(TOKEN_RIGHT_PAREN, ")", 1);
                if (closing == NULL) {
                    printf("Error creating the closing parenthesis token.\n");
                    return -1;
                }
                closing->next = func->next->next;
                func->next = closing;

                return 1;

            }
        }
        else {
            printf("Function call \"%s\" must be followed by required parameters.\n", (*cur)->value);
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Handles negatives
 * 
 * @retval -1: Error, new tokens couldn't be created
 * @retval 0: No negative to handle
 * @retval 1: Negative handled
 * 
 * @param ptr Pointer to the pointer to the current Token
 * @param prev Pointer to the previous Token
 * @return int Error code
 */
static int handleNegatives(Token *cur, Token *prev) {
    // Determines if a '-' is in place
    if (cur->type != TOKEN_OPERATOR && strcmp(cur->value, "-")) return 0;

    // Outlines cases for following negative handling
    // (ie determines this is a negative and not a subtraction)
    if (cur->next == NULL || (cur->next->type != TOKEN_IDENTIFIER && cur->next->type != TOKEN_NUMBER && cur->next->type != TOKEN_FUNC_CALL_PLACEHOLDER)) return 0;
    if (prev != NULL && prev->type != TOKEN_OPERATOR && prev->type != TOKEN_LEFT_PAREN && prev->type != TOKEN_SEPARATOR) return 0;

    Debug(0, "Creating -1 and multiplication tokens.\n");
    Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
    if (mult == NULL) {
        printf("Error handling negative.\n");
        return -1;
    }

    cur->type = TOKEN_NUMBER;
    free(cur->value);
    cur->value = strdup("-1");

    Token *number = cur->next;

    // Modifies Token list to include new Tokens
    cur->next = mult;
    mult->next = number;
    
    return 1;
}


void handleLocalVariables(Token **ptr, Environment *localEnv) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    Debug(0, "\nRechecking identifiers against local variables.\n");
    Token *cur = *ptr;
    Token *prev = NULL;
    
    while (cur != NULL) {
        if (cur->type == TOKEN_IDENTIFIER) {
            int max = 0;
            Component *cmp;
            char *id = cur->value;

            for (int i = 0; id[i] != '\0'; i ++) {
                // Adds the end of string char to only select a part of the buffer
                char temp = id[i + 1];
                id[i + 1] = '\0';
                Component *local = searchEnvironment(localEnv, id);
                Component *global = searchEnvironment(env, id);
                id[i + 1] = temp;

                // Any subsequent iterations will always yeild a larger char size if found, so no check required
                // If local var is found take it 
                if (local != NULL) {
                    max = strlen(local->identifier);
                    cmp = local;
                    continue;
                }
                
                // If global var is found take it
                if (global != NULL) {
                    max = strlen(global->identifier);
                    cmp = global;
                    continue;
                }

            }

            // If smaller identifer than the current one is found, partition it into two new identifier tokens
            if (max != strlen(cur->value) && max != 0) {
                // create new tokens to split
                Token *left = createToken(TOKEN_IDENTIFIER, cmp->identifier, max);
                Token *right = createToken(TOKEN_IDENTIFIER, id + max, strlen(id) - max);

                left->next = right;
                right->next = cur->next;

                if (prev != NULL) prev->next = left;
                else *ptr = left;

                free(cur->value);
                free(cur);

                prev = right;
                cur = left;
            }

        }

        prev = cur;
        cur = cur->next;
    }
}


int lex(Token** head) {
    Config *config = GLOBALCONTEXT->config;
    Debug(0, "\nLexing Tokens\n");

    Token **ptr = head;
    Token *prev = NULL;

    int openParenthesis = 0;

    while (*ptr != NULL) {
        if (handleNegatives(*ptr, prev) == -1) return 0;
        if (handleImplicitMul(*ptr, prev) == -1) return 0;
        if (handleExponentRewrite(ptr, prev) == -1) return 0;
        if (checkInvalidBinop(*ptr, prev) == -1) return 0;
        if (handleFunctionParens(ptr) == -1) return 0;

        // Counts open parenthesis
        if ((*ptr)->type == TOKEN_LEFT_PAREN) {
            // Empty parens, what are we doing gang
            if ((*ptr)->next->type == TOKEN_RIGHT_PAREN) {
                printf("Empty parenthesis.\n");
                Debug(0, "Empty parens\n");
                Debug(1, printTokens(*head));
                return 0;
            }
            openParenthesis ++;
        }

        if ((*ptr)->type == TOKEN_RIGHT_PAREN) {
            if (openParenthesis <= 0) {
                printf("Mismatched parenthesis.\n");
                return 0;
            }

            openParenthesis --;
        }

        prev = *ptr;
        ptr = &((*ptr)->next);
    }

    if (openParenthesis > 0) {
        printf("Mismatched parenthesis.\n");
        return 0;
    } 

    Debug(0, "Updated Tokens\n");
    Debug(1, printTokens(*head));

    return 1;

}