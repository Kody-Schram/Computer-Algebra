#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "core/common.h"
#include "core/context.h"
#include "core/execution/executor.h"
#include "core/parsing/parser_types.h"
#include "core/utils/log.h"
#include "core/utils/expr_utils.h"

#include "core/parsing/codegen/tokenizer.h"
#include "core/parsing/codegen/normalizer.h"
#include "core/parsing/codegen/ast.h"
#include "core/parsing/parser_utils.h"


#define DEFAULT_PARAMETERS_SIZE 3

typedef enum {
    IDENTIFIER,
    PARAMETERS, 
    BODY
} FunctionComponent;


// parseFunctionCalls fragments the list and is now responsible for cleaning up its mess
static PARSER_RESULT parseFunctionCalls(Token **head) {
	Debug(0, "parsing function calls\n");
	Token *cur = *head;
	Token *placeholderToken= NULL;
	Token *funcPrev = NULL;

	while (cur != NULL) {
		if (cur->type != TOKEN_FUNC_CALL_PLACEHOLDER) {
			funcPrev = cur;
			cur = cur->next;

			continue;
		}

		placeholderToken = cur;

		// skips past opening ( and frees it
		cur = cur->next->next; 
		free(placeholderToken->next->value);
		free(placeholderToken->next);

		Token *tmp = NULL;

		uint16_t depth = 0;
		Token *paramHead = cur; 
		Function *func = placeholderToken->cmp->func;

		Token **params = calloc(1, sizeof(Token *) * func->nParameters);
		Expression **paramExprs = calloc(1, sizeof(Expression *) * func->nParameters);
		if (params == NULL || paramExprs == NULL) {
			free(params);
			free(paramExprs);

			perror("Error parsing function call");
			return PARSER_ERROR;
		}

		uint32_t nParams = 0;

		while (!(depth == 0 && cur->next->type == TOKEN_RIGHT_PAREN)) {
			if (cur->next->type == TOKEN_LEFT_PAREN) depth ++;
			else if (cur->next->type == TOKEN_RIGHT_PAREN) depth --;

			if (depth == 0 && cur->next->type == TOKEN_SEPARATOR) {
				// Skips seperator, frees the seperator, and severs parameter for overall list, then adds it to parameters list
				tmp = cur->next->next;

				free(cur->next->value);
				free(cur->next);
				cur->next = NULL;

				cur = tmp;

				params[nParams] = paramHead;
				nParams ++;

				paramHead = cur;

				continue;
			}

			cur = cur->next;
		}

		// Skips seperator, frees the seperator, and severs parameter for overall list, then adds it to parameters list
		tmp = cur->next->next;

		free(cur->next->value);
		free(cur->next);
		cur->next = NULL;

		cur = tmp;

		params[nParams] = paramHead;
		nParams ++;


		for (uint32_t i = 0; i < nParams; i ++) {
			Debug(0, "Recursively parsing calls\n");
			// Recursively parses calls
			PARSER_RESULT callOut = parseFunctionCalls(&params[i]);
			if (callOut != PARSER_SUCCESS) goto error;

			RPNList *rpn = shuntingYard(params[i]);	
			if (rpn == NULL) goto error;
			
			Expression *expr = expressionFromRPN(rpn);
			free(rpn->items);
			free(rpn);
			if (expr == NULL) goto error;

			paramExprs[i] = expr;
		}

		
		// Creates new function call token
		Debug(0, "Creating new function call token.\n");
		Token *callToken = calloc(1, sizeof(Token));
		if (callToken == NULL) goto error;
		
		callToken->type = TOKEN_FUNC_CALL;
		callToken->next = cur;

		Expression *callExpr = dummyExpression(EXPRESSION_FUNCTION_CALL);
		if (callExpr == NULL) goto error;
		callExpr->nInputs = nParams;
		callExpr->inputs = paramExprs;
		callExpr->cmp = placeholderToken->cmp;
		Debug(0, "end of call nparams %d\n", nParams);

		callToken->finalizedCall = callExpr;


		Debug(0, "Replacing old call token with new one.\n");

		if (funcPrev != NULL) funcPrev->next = callToken;
		else *head = callToken;

		free(placeholderToken);

		for (uint32_t i = 0; i < nParams; i ++) {
			freeTokens(params[i]);
		}

		free(params);

		funcPrev = callToken;

		continue;


		error:
			for (uint32_t i = 0; i < nParams; i ++) {
				deepFreeTokens(params[i]);
				freeExpression(paramExprs[i]);
			}

			free(params);
			free(paramExprs);

			return PARSER_ERROR;
	}

	return PARSER_SUCCESS;
}


static PARSER_RESULT parseFunctionAssignment(Token *head, bool hasCalls) {
    char *identifier = NULL;
    RPNList *rpn = NULL;
    Expression *expr = NULL;
    Function *function = NULL;

    Debug(0, "Parsing function definition.\n");
    FunctionComponent component = IDENTIFIER;

    uint32_t size = DEFAULT_PARAMETERS_SIZE;
    char **parameters = malloc(sizeof(char *) * size);
    if (parameters == NULL) goto error;
    uint32_t nParameters = 0;
    
    Token *cur = head;
    Token *asgn = NULL;
    while (cur != NULL && component != BODY) {
        if (component == IDENTIFIER) {
            // Switches to PARAMETERS if ':' is found
            if (cur->type == TOKEN_ASSIGNMENT) {
                Debug(0, "Moving to function parameters.\n");
                component = PARAMETERS;
            } else if (cur->type != TOKEN_IDENTIFIER) {
                printf("Invalid token: within function identifier.\n");
                goto syntax_error;
            } else {
                if (identifier != NULL) {
                    printf("Invalid function declaration.\n");
                    goto syntax_error;
                }
                identifier = cur->value;
                if (identifier == NULL) {
                    perror("Error parsing function assignment");
                    goto syntax_error;
                }
            }
        } else if (component == PARAMETERS) {
            // Switches to BODY if '->' is found
            if (cur->type == TOKEN_MAPPING) {
                Debug(0, "Moving to function body.\n");
                if (nParameters == 0) {
                    printf("No parameters were passed. If this is intentional, define a variable instead.\n");
                    goto syntax_error;
                }
                asgn = cur;
                component = BODY;
            } else if (cur->type != TOKEN_IDENTIFIER && cur->type != TOKEN_SEPARATOR) {
                printf("Invalid token: within function parameters.\n");

                goto syntax_error;
            } else if (cur->type == TOKEN_IDENTIFIER) {
                Debug(0, "Adding function parameter '%s'\n", cur->value);
                if (nParameters >= size) {
                    size *= 2;
                    char **temp = realloc(parameters, sizeof(char *) * size);
                    if (temp == NULL) goto error;
                    
                    parameters = temp;
                }
                
                parameters[nParameters] = cur->value;
                nParameters ++;
            }
        }

        cur = cur->next;
    }

	if (component != BODY) {
		printf("Invalid function definition\n");
		goto syntax_error;
	}

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
	PARSER_RESULT result = handleLocalVariables(&asgn, parameters, nParameters);
	if (result != PARSER_SUCCESS) {
		if (result == PARSER_ERROR) goto error;
		goto syntax_error;
	}

	result = normalize(&asgn->next);
	if (result != PARSER_SUCCESS) {
		if (result == PARSER_SYNTAX_ERROR) goto syntax_error;
		goto error;
	}

	if (hasCalls) {
		result = parseFunctionCalls(&asgn->next);
		if (result != PARSER_SUCCESS) {
			if (result == PARSER_SYNTAX_ERROR) goto syntax_error;
			goto error;
		}
	}

    rpn = shuntingYard(asgn->next);
    if (rpn == NULL) goto error;

    // Generate ast for body
    expr = expressionFromRPN(rpn);
    if (expr == NULL) goto error;
    
	EXECUTOR_RESULT e_result = execute(&expr, false);
	

    Debug(1, printExpression(expr));

    // Add to function table
    function = malloc(sizeof(Function));
    if (function == NULL) goto error;

    function->parameters = parameters;
    function->nParameters = nParameters;
    function->type = DEFINED;
    function->definition = expr;

    if (!bindComponent(GLOBALCONTEXT->env, COMP_FUNCTION, identifier, function)) goto error;

    freeTokens(head);
    free(rpn->items);
    free(rpn);

	Debug(0, "Successfully parsed function assignment, %d\n", function->nParameters);

    return PARSER_SUCCESS;

	
	syntax_error:
		deepFreeTokens(head);
        if (parameters != NULL) {
            for (int i = 0; i < nParameters; i ++) free(parameters[i]);
        }
        free(parameters);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        if (expr != NULL) freeExpression(expr);
        free(function);
		return PARSER_SYNTAX_ERROR;

    
    error:
		deepFreeTokens(head);
        free(identifier);
        if (parameters != NULL) {
            for (int i = 0; i < nParameters; i ++) free(parameters[i]);
        }
        free(parameters);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        if (expr != NULL) freeExpression(expr);
        free(function);

        return PARSER_ERROR;
}


static PARSER_RESULT parseAssignment(Token *head, bool hasCalls) {
    char *identifier = NULL;
    RPNList *rpn = NULL;
	Expression *expr = NULL;

    Token *cur = head;
    Token *asgn = NULL;
    while (cur != NULL && asgn == NULL) {
        if (identifier == NULL) {
            if (cur->type == TOKEN_IDENTIFIER) {
                identifier = cur->value;
                if (identifier == NULL) {
                    perror("Error parsing variable assignment");
                    goto syntax_error;
                }
            } else {
                printf("Invalid token '%s' in variable identifier.\n", cur->value);
                goto syntax_error;
            }
        } else if (cur->type == TOKEN_ASSIGNMENT) {
            asgn = cur;
        }

        cur = cur->next;
    }

	if (hasCalls) {
		PARSER_RESULT callOut = parseFunctionCalls(&asgn->next);
		if (callOut != PARSER_SUCCESS) {
			if (callOut == PARSER_ERROR) goto error;
			goto syntax_error;
		}
	}

    rpn = shuntingYard(asgn->next);
    if (rpn == NULL) goto error;

    // Generate ast for body
    expr = expressionFromRPN(rpn);
    if (expr == NULL) goto error;

    
    // ===============================================
    // Run expression through simplification step here
    // ===============================================
    
    Debug(1, printExpression(expr));
    
    if (!bindComponent(GLOBALCONTEXT->env, COMP_VARIABLE, identifier, expr)) goto error;

    freeTokens(head);
    free(rpn->items);
    free(rpn);

    return PARSER_SUCCESS;
	

	syntax_error:
        free(identifier);
        deepFreeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeExpression(expr);

		return PARSER_SYNTAX_ERROR;

    error:
        free(identifier);
        deepFreeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeExpression(expr);
        
		return PARSER_ERROR;
}


PARSER_RESULT parse(char *buffer, Expression **expr) {
    Info(0, "\nParsing: '%s'\n", buffer);
    
    Token *head = NULL;
    RPNList *rpn = NULL;
    
    head = tokenize(buffer);
    if (head == NULL) return PARSER_ERROR;

	bool hasFuncCalls = head->hasFunctionCalls;
	bool hasFuncAssignment = head->hasFuncAssignment;
	bool hasVarAssignment = head->hasVarAssignment;

	PARSER_RESULT result = normalize(&head);
	if (result != PARSER_SUCCESS) {
		if (result == PARSER_ERROR) goto error;
		goto syntax_error;
	}

	Debug(0, "Normalizer out\n");
	Debug(1, printTokens(head));

    if (hasVarAssignment) {
        if (hasFuncAssignment) return parseFunctionAssignment(head, hasFuncCalls);
        return parseAssignment(head, hasFuncCalls);
    }

	if (hasFuncCalls) {
		result = parseFunctionCalls(&head);
		if (result != PARSER_SUCCESS) return result; 
	}

    rpn = shuntingYard(head);
    if (rpn == NULL) goto error;

    *expr = expressionFromRPN(rpn);

    free(rpn->items);
    free(rpn);
    freeTokens(head);

	return PARSER_SUCCESS;

	syntax_error:
    	deepFreeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
		return PARSER_SYNTAX_ERROR;

    error:
        deepFreeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
      	return PARSER_ERROR;
}
