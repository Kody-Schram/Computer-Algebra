#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "normalizer.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/parsing/parser_types.h"
#include "core/utils/log.h"
#include "core/parsing/parser_utils.h"

/**
 * @brief Adds * where it is implied through standard math notation
 * 
 * @retval 0: Error
 * @retval 1: Success
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
                perror("Error in implicit multiplication");
                return 0;
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
                perror("Error in implicit multiplication");
                return 0;
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
                perror("Error in implicit multiplication");
                return 0;
            }

            prev->next = mult;
            mult->next = cur;
			return 1; 
		}
    }

    return 1;
}

/**
 * @brief Replaces ** tokens with a ^ token
 * 
 * @retval 0: Error
 * @retval 1: Success
 * 
 * @param cur Current node in the list
 * @param prev Previous node in the list
 * @return int Return code
 */
static int handleExponentRewrite(Token **cur, Token *prev) {
    if ((*cur) == NULL || (*cur)->type != TOKEN_OPERATOR || (*cur)->op->symbol != '*') return 1;
	if ((*cur)->next == NULL || (*cur)->next->type != TOKEN_OPERATOR || (*cur)->next->op->symbol != '*') return 1; 

	Debug(0, "Rewriting exponent\n"); 
	
	Token *exponent = createToken(TOKEN_OPERATOR, "^", 1);
    if (exponent == NULL) {
		perror("Error in exponent rewrite"); 
		return 0; 
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

/** @brief Checks to ensure binary operators valid
 * @retval 0: Error 
 * @retval 1: Success
 * 
 * @param cur Current node in the list
 * @param prev Previous node in the list
 * @return int Return code
 */
static int checkInvalidBinop(const Token *cur, const Token *prev) {
    if (cur->type != TOKEN_OPERATOR) return 1;
    if (cur->next == NULL) {
        printf("Operator must be followed by another token.\n");
        return 0;
    }

    if (prev == NULL) {
        printf("Operator must be preceeded by another token.\n");
        return 0;
    }

    // If next op is -, will recheck later, may be a negative number instead of a minus
    if (cur->next->type == TOKEN_OPERATOR && cur->next->value[0] == '-') return 0;

    // Handles invalid negative signs
    if (cur->value[0] == '-' && (cur->next == NULL || cur->next->type == TOKEN_OPERATOR)) {
        printf("Invalid operation \"%s %s %s\".\n", prev->value, cur->value, cur->next->value);
        return 0;
    }

    if (cur->next->type != TOKEN_IDENTIFIER && cur->next->type != TOKEN_NUMBER && cur->next->type != TOKEN_FUNC_CALL_PLACEHOLDER && cur->next->type != TOKEN_LEFT_PAREN) {
        printf("Invalid operation \"%s %s %s\".\n", prev->value, cur->value, cur->next->value);
        return 0;
    }
    
    return 1;
}

/**
 * @brief Adds parenthesis around first term after function call if none are found
 * 
 * @retval 0: Error 
 * @retval 1: Success
 * 
 * @param cur Current node in the list
 * @return int Return code
 */
static int handleFunctionParens(Token *cur) {
	if (cur == NULL || cur->type != TOKEN_FUNC_CALL_PLACEHOLDER) return 1;

	// Assume since theres opening paren that the parameters are explicit
	if (cur->next == NULL || cur->next->type == TOKEN_LEFT_PAREN) return 1;

	Debug(0, "handling function parens\n");
	Token *func = cur;
	Token *opening = NULL;
	Token *closing = NULL;

	opening = createToken(TOKEN_LEFT_PAREN, "(", 1);
	if (opening == NULL) goto error;

	while (cur->next != NULL && cur->next->type != TOKEN_OPERATOR) cur = cur->next;

	closing = createToken(TOKEN_RIGHT_PAREN, ")", 1);
	if (closing == NULL) goto error;

	closing->next = cur->next;
	cur->next = closing;

	
	if (opening != NULL) opening->next = func->next;
	func->next = opening;

	return 1;

	error:
		perror("Error handling function parentheses");
		freeTokens(opening);
		freeTokens(closing);

		return 0;
}

/**
 * @brief Handles negatives
 * 
 * @retval 0: Error
 * @retval 1: Success
 * 
 * @param ptr Pointer to the pointer to the current Token
 * @param prev Pointer to the previous Token
 * @return int Error code
 */
static int handleNegative(Token *cur, Token *prev) {
	if (cur->type != TOKEN_OPERATOR || cur->op->symbol != '-' || prev == NULL) return 1;
	if (prev->type != TOKEN_LEFT_PAREN && prev->type != TOKEN_OPERATOR && prev->type != TOKEN_ASSIGNMENT) return 1; // is subtraction

	Token *negative = NULL;
	Token *mult = NULL;

	negative = createToken(TOKEN_NUMBER, "-1", 2);
	if (negative == NULL) goto error;

	mult = createToken(TOKEN_OPERATOR, "*", 1);
	if (mult == NULL) goto error;

	negative->next = mult;
	mult->next = cur;
	prev->next = negative;

	return 1;

	error:
		perror("Error handling negative");
		freeTokens(negative);
		freeTokens(mult);

		return 0;
}


int handleLocalVariables(Token **ptr, char **parameters, int nParameters) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    Debug(0, "\nRechecking identifiers against local variables.\n");
    Token *cur = *ptr;
    Token *prev = NULL;
    
    while (cur != NULL) {
        if (cur->type == TOKEN_IDENTIFIER) {
            int max = 0;
            char *identifier;
            char *id = cur->value;

            for (int i = 0; id[i] != '\0'; i ++) {
                // Adds the end of string char to only select a part of the buffer
                char temp = id[i + 1];
                id[i + 1] = '\0';
                char *local = NULL;
                for (int i = 0; i < nParameters; i ++) {
                    if (!strcmp(id, parameters[i])) local = parameters[i];
                }
                Component *global = searchEnvironment(env, id);
                id[i + 1] = temp;

                // Any subsequent iterations will always yeild a larger char size if found, so no check required
                // If local var is found take it 
                if (local != NULL) {
                    max = strlen(local);
                    identifier = local;
                    continue;
                }
                
                // If global var is found take it
                if (global != NULL) {
                    max = strlen(global->identifier);
                    identifier = global->identifier;
                    continue;
                }

            }

            // If smaller identifer than the current one is found, partition it into two new identifier tokens
            if (max != strlen(cur->value) && max != 0) {
                // create new tokens to split
                Token *left = createToken(TOKEN_IDENTIFIER, identifier, max);
                if (left == NULL) {
                    perror("Error handling local variables");
                    return 0;
                }

                Token *right = createToken(TOKEN_IDENTIFIER, id + max, strlen(id) - max);
                if (right == NULL) {
                    perror("Error handling local variables");
                    free(left);
                    return 0;
                }

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

    return 1;
}


int normalize(Token** head) {
    Debug(0, "\nNormalizing Tokens\n");

    Token **ptr = head;
    Token *prev = NULL;

    int openParenthesis = 0;

    while (*ptr != NULL) {
        if (!handleNegative(*ptr, prev)) return 0;
        if (!handleFunctionParens(*ptr)) return 0;
        if (!handleImplicitMul(*ptr, prev)) return 0;
        if (!handleExponentRewrite(ptr, prev)) return 0;
        if (!checkInvalidBinop(*ptr, prev)) return 0;

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
