#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "normalizer.h"
#include "core/context.h"
#include "core/context/environment.h"
#include "core/parsing/parser.h"
#include "core/parsing/parser_types.h"
#include "core/utils/log.h"
#include "core/parsing/parser_utils.h"

/*
 * Inserts implicit multiplication if found
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success or no work done
 *
 */
static PARSER_RESULT handleImplicitMul(Token *cur, Token *prev) {
    // Handle implicit multiplcation with left bracket or function
    // x(x-1) => x*(x-1)
    if ((cur->type == TOKEN_LEFT_PAREN || cur->type == TOKEN_FUNC_CALL_PLACEHOLDER) && prev != NULL) {
        //printf("checking for left paren and function\n");
        if (prev->type == TOKEN_NUMBER || prev->type == TOKEN_IDENTIFIER) {
            Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
            if (mult == NULL) {
                perror("Error in implicit multiplication");
                return PARSER_ERROR;
            }

            prev->next = mult;
            mult->next = cur;

            return PARSER_SUCCESS;
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
                return PARSER_ERROR;
            }

            mult->next = nextToken;
            cur->next = mult;

            return PARSER_SUCCESS;
        }
    }
    // Handle constant and identifier multiplication
    // 2x => 2*x
    else if (cur->type == TOKEN_IDENTIFIER && prev != NULL) {
        if (prev->type == TOKEN_NUMBER || prev->type == TOKEN_IDENTIFIER) {

            Token *mult = createToken(TOKEN_OPERATOR, "*", 1);
            if (mult == NULL) {
                perror("Error in implicit multiplication");
                return PARSER_ERROR;
            }

            prev->next = mult;
            mult->next = cur;
			return PARSER_SUCCESS; 
		}
    }

    return PARSER_SUCCESS;
}

/*
 * Rewrites ** into ^ if found
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success or no work done
 *
 */
static PARSER_RESULT handleExponentRewrite(Token **cur) {
    if ((*cur)->type != TOKEN_OPERATOR || (*cur)->op->symbol != '*') return PARSER_SUCCESS;
	if ((*cur)->next == NULL || (*cur)->next->type != TOKEN_OPERATOR || (*cur)->next->op->symbol != '*') return PARSER_SUCCESS; 

	Debug(0, "Rewriting exponent\n");

	Token *exponent = createToken(TOKEN_OPERATOR, "^", 1);
    if (exponent == NULL) {
		perror("Error in exponent rewrite"); 
		return PARSER_ERROR; 
	}

	exponent->next = (*cur)->next->next;
	
	free((*cur)->next);
    free(*cur);

    *cur = exponent;

    return PARSER_SUCCESS;
}

/*
 * Checks for invalid binary operation syntax
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success or no work done
 *
 */
static PARSER_RESULT checkBinop(const Token *cur, const Token *prev) {
    if (cur->type != TOKEN_OPERATOR) return PARSER_SUCCESS; // no work to do
    if (cur->next == NULL) {
        printf("Operator must be followed by another token.\n");
        return PARSER_SYNTAX_ERROR;
    }

    if (prev == NULL) {
        printf("Operator must be preceeded by another token.\n");
        return PARSER_SYNTAX_ERROR;
    }

    // If next op is -, will recheck later, may be a negative number instead of a minus
    if (cur->next->type == TOKEN_OPERATOR && cur->next->op->symbol == '-') return PARSER_SUCCESS;

    // Handles invalid negative signs
    if (cur->op->symbol == '-' && (cur->next == NULL || cur->next->type == TOKEN_OPERATOR)) {
        if (prev->type != TOKEN_OPERATOR && cur->next->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%s %c %s\".\n", prev->value, cur->op->symbol, cur->next->value);
		} else if (prev->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%s %c %c\".\n", prev->value, cur->op->symbol, cur->next->op->symbol);
		} else if (cur->next->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%c %c %s\".\n", prev->op->symbol, cur->op->symbol, cur->next->value);
		}

        return PARSER_SYNTAX_ERROR;
    }

    if (cur->next->type != TOKEN_IDENTIFIER && cur->next->type != TOKEN_NUMBER && cur->next->type != TOKEN_FUNC_CALL_PLACEHOLDER && cur->next->type != TOKEN_LEFT_PAREN) {
        if (prev->type != TOKEN_OPERATOR && cur->next->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%s %c %s\".\n", prev->value, cur->op->symbol, cur->next->value);
		} else if (prev->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%s %c %c\".\n", prev->value, cur->op->symbol, cur->next->op->symbol);
		} else if (cur->next->type != TOKEN_OPERATOR) {
			printf("Invalid operation \"%c %c %s\".\n", prev->op->symbol, cur->op->symbol, cur->next->value);
		}

        return PARSER_SYNTAX_ERROR;
    }
    
    return PARSER_SUCCESS;
}

/*
 * Inserts implicit function parenthesis if needed.
 * Consumes tokens until first user defined operator.
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success or no work done
 *
 */
static PARSER_RESULT handleFunctionParens(Token *cur) {
	if (cur->type != TOKEN_FUNC_CALL_PLACEHOLDER) return PARSER_SUCCESS;

	// Assume since theres opening paren that the parameters are explicit
	if (cur->next == NULL || cur->next->type == TOKEN_LEFT_PAREN) return PARSER_SUCCESS;

	Debug(0, "Adding implicit function parens\n");

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

	return PARSER_SUCCESS;

	error:
		perror("Error handling function parentheses");
		free(opening->value);
		free(opening);
		free(closing->value);
		free(closing);

		return PARSER_ERROR;
}

/*
 * Handles converting some subtraction operations into negation
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success or no work done
 *
 */
static PARSER_RESULT handleNegative(Token **cur, Token *prev) {
	if ((*cur)->type != TOKEN_OPERATOR || (*cur)->op->symbol != '-') return PARSER_SUCCESS;
	if (prev != NULL && prev->type != TOKEN_LEFT_PAREN && prev->type != TOKEN_OPERATOR && prev->type != TOKEN_ASSIGNMENT) return PARSER_SUCCESS; // is subtraction
	
	Debug(0, "Rewriting subtraction as negation\n");

	Token *negative = NULL;
	Token *mult = NULL;

	negative = createToken(TOKEN_NUMBER, "-1", 2);
	if (negative == NULL) goto error;

	mult = createToken(TOKEN_OPERATOR, "*", 1);
	if (mult == NULL) goto error;


	negative->next = mult;
	mult->next = (*cur)->next;
	free(*cur);
	if (prev == NULL) *cur = negative;
	else prev->next = negative;


	return PARSER_SUCCESS;

	error:
		perror("Error handling negative");
		free(negative->value);
		free(negative);
		free(mult->value);
		free(mult);

		return PARSER_ERROR;
}

/*
 * Handles rewriting local variables within function definition again function parameters
 *
 * Return values:
 * -1: System error
 * 0: Syntax error
 * 1: Success or no work done
 *
 */
PARSER_RESULT handleLocalVariables(Token **ptr, char **parameters, int nParameters) {
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
                Component const *global = searchEnvironment(env, id);
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
                    return PARSER_ERROR;
                }

                Token *right = createToken(TOKEN_IDENTIFIER, id + max, strlen(id) - max);
                if (right == NULL) {
                    perror("Error handling local variables");
					free(left->value);
                    free(left);
                    return PARSER_ERROR;
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

    return PARSER_SUCCESS;
}


static PARSER_RESULT checkFunctionCall(Token *cur) {
	if (cur->type != TOKEN_FUNC_CALL_PLACEHOLDER) return PARSER_SUCCESS;

	Token *callToken = cur;
	Debug(0, "Checking call to '%s'\n", cur->cmp->identifier);

	uint32_t nParams = 0;
	uint16_t depth = 0;
	bool isParam = false;

	cur = cur->next->next;

	while (cur != NULL && !(depth == 0 && cur->type == TOKEN_RIGHT_PAREN)) {
		if (cur->type == TOKEN_LEFT_PAREN) depth ++;
		else if (cur->type == TOKEN_RIGHT_PAREN) depth --;

		if ((depth == 0 && cur->type == TOKEN_SEPARATOR) || (depth == 1 && cur->type == TOKEN_RIGHT_PAREN)) {
			if (!isParam) {
				printf("Invalid function call syntax for '%s', invalid parameter.\n", callToken->cmp->identifier);
				return PARSER_SYNTAX_ERROR;	
			}

			nParams ++;
		}

		else if (cur->type != TOKEN_LEFT_PAREN && cur->type != TOKEN_RIGHT_PAREN && cur->type != TOKEN_SEPARATOR) isParam = true;
		
		cur = cur->next;
	}

	if (!isParam) {
		printf("Invalid function call syntax for '%s', invalid parameter.\n", callToken->cmp->identifier);
		return PARSER_SYNTAX_ERROR;	
	}

	nParams ++;

	if (nParams != callToken->cmp->func->nParameters) {
		if (nParams == 1) printf("Function '%s' takes %d parameters, %d was provided\n",
				callToken->cmp->identifier, callToken->cmp->func->nParameters, nParams);

		else printf("Function '%s' takes %d parameters, %d were provided\n", callToken->cmp->identifier, callToken->cmp->func->nParameters, nParams);
		return PARSER_SYNTAX_ERROR;
	}

	return PARSER_SUCCESS;
}

/**
 * Normalizes input tokens
 *
 * Return values:
 * -1: System error
 *  0: Syntax error
 *  1: Success
 *
 *	Token **head: pointer to token linked list
 *
 */
PARSER_RESULT normalize(Token** head) {
    Debug(0, "\nNormalizing Tokens\n");

    Token **ptr = head;
    Token *prev = NULL;
	PARSER_RESULT result = PARSER_SUCCESS;

    int openParenthesis = 0;

    while (*ptr != NULL) {
        if ((result = handleNegative(ptr, prev)) != PARSER_SUCCESS) return result;
        if ((result = handleFunctionParens(*ptr)) != PARSER_SUCCESS) return result;
        if ((result = handleImplicitMul(*ptr, prev)) != PARSER_SUCCESS) return result;
        if ((result = handleExponentRewrite(ptr)) != PARSER_SUCCESS) return result;
        if ((result = checkBinop(*ptr, prev)) != PARSER_SUCCESS) return result;
		if ((result = checkFunctionCall(*ptr)) != PARSER_SUCCESS) return result;
		// Counts open parenthesis 
		if ((*ptr)->type == TOKEN_LEFT_PAREN) { 
			//Empty parens, what are we doing gang 
			if ((*ptr)->next->type == TOKEN_RIGHT_PAREN) {
                printf("Empty parenthesis.\n");
                return PARSER_SYNTAX_ERROR;
            }
            openParenthesis ++;
        }

        if ((*ptr)->type == TOKEN_RIGHT_PAREN) {
            if (openParenthesis <= 0) {
                printf("Mismatched parenthesis.\n");
                return PARSER_SYNTAX_ERROR;
            }

            openParenthesis --;
        }

        prev = *ptr;
        ptr = &((*ptr)->next);
    }

    if (openParenthesis > 0) {
        printf("Mismatched parenthesis.\n");
        return PARSER_SYNTAX_ERROR;
    } 

    Debug(1, printTokens(*head));

    return PARSER_SUCCESS;

}
