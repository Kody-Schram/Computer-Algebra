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
 * @return int Error code
 */
static int checkInvalidBinop(Token *cur, Token *prev) {
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
    if ((*cur)->type == TOKEN_FUNC_CALL) {
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
static int handleNegatives(Token **ptr, Token *prev) {
    Token *cur = *ptr;

    // Determines if a '-' is in place
    if (cur->type != TOKEN_OPERATOR) return 0;
    if (strcmp(cur->value, "-")) return 0;

    // Outlines cases for following negative handling
    // (ie determines this is a negative and not a subtraction)
    if (prev != NULL && prev->type != TOKEN_OPERATOR && prev->type != TOKEN_LEFT_PAREN) return 0;
    
    //printf("Creating -1 and multiplication tokens.\n");

    Token *negative_one = createToken(TOKEN_NUMBER, "-1", 2);
    Token *mult = createToken(TOKEN_OPERATOR, "*", 1);

    if (negative_one == NULL || mult == NULL) {
        printf("Error handling negatives.\n");
        return -1;
    }

    // Modifies Token list to include new Tokens
    negative_one->next = mult;
    mult->next = cur->next;

    if (prev != NULL) prev->next = negative_one;
    free(cur->value);
    free(cur);

    **ptr = *negative_one;

    return 1;
}


static int handleAssignment(Token *cur) {
    int invalid = 0;

    while (cur != NULL) {
        if (cur->type != TOKEN_IDENTIFIER &&
            cur->type != TOKEN_SEPERATOR &&
            cur->type != TOKEN_FUNC_DEF) invalid = 1;

        if (invalid && cur->type == TOKEN_ASSIGNMENT) {
            printf("Invalid assignment.\n");
            return 0;
        }

        cur = cur->next;
    }

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
            Debug(0, "Rechecking identifier %s\n", cur->value);
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

                // If local var is found take it 
                if (local != NULL) {
                    Debug(0, "Local identifier %s found\n", local->identifier);
                    max = strlen(local->identifier);
                    cmp = local;
                    continue;
                }
                
                // Any subsequent iteration will always yeild a larger char size if found, so no check required
                if (global != NULL) {
                    Debug(0, "Global identifier %s found\n", global->identifier);
                    max = strlen(global->identifier);
                    cmp = global;
                    continue;
                }

            }

            // If smaller identifer than the current one is found, partition it into two new identifier tokens
            if (max != strlen(cur->value) && max != 0) {
                // create new tokens to split
                Token *left = createToken(TOKEN_IDENTIFIER, cmp->identifier, max);
                Debug(0, "string: '%s', right: '%s', right len: %d\n", cur->value, id + max, strlen(id) - max);
                Token *right = createToken(TOKEN_IDENTIFIER, id + max, strlen(id) - max);

                //printToken(prev);
                printToken(left);
                printToken(right);

                left->next = right;
                right->next = cur->next;

                if (prev != NULL) prev->next = left;
                else *ptr = left;

                free(cur->value);
                free(cur);

                prev = right;
                cur = left;

                Debug(0, "Intermediate token list\n");
                printTokens(*ptr);
            }

        }

        prev = cur;
        cur = cur->next;
    }

    if (config->LOG_LEVEL >= DEBUG) {
        fprintf(config->LOG_STREAM, "Updated for local vars\n");
        printTokens(*ptr);
    }
}

Token *lex(Token* head) {
    Config *config = GLOBALCONTEXT->config;
    Debug(0, "\nLexing Tokens\n");

    Token *cur = head;
    Token *prev = NULL;

    int openParenthesis = 0;

    while (cur != NULL) {
        // printf("Current Token: <%u, %s>\n", cur->type, cur->value);
        if (handleNegatives(&cur, prev) == -1) return NULL;
        if (handleImplicitMul(cur, prev) == -1) return NULL;
        if (handleExponentRewrite(&cur, prev) == -1) return NULL;
        if (checkInvalidBinop(cur, prev) == -1) return NULL;
        if (handleFunctionParens(&cur) == -1) return NULL;

        // Counts open parenthesis
        if (cur->type == TOKEN_LEFT_PAREN) {
            openParenthesis ++;
        }

        if (cur->type == TOKEN_RIGHT_PAREN) {
            if (openParenthesis <= 0) {
                printf("Mismatched parenthesis.\n");
                return NULL;
            }

            openParenthesis --;
        }

        prev = cur;
        cur = cur->next;
    }

    if (openParenthesis > 0) {
        printf("Mismatched parenthesis.\n");
        return NULL;
    } 

    if (!handleAssignment) return NULL;
    
    if (config->LOG_LEVEL >= DEBUG) {
        fprintf(config->LOG_STREAM, "Updated Tokens.\n");
        printTokens(head);
    }

    return head;

}