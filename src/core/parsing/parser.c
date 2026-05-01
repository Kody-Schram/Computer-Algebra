#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/parsing/parser_types.h"
#include "core/utils/log.h"
#include "core/utils/type_utils.h"

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


static int containsFunctionAssignment(Token *head) {
    Token *cur = head;
    while (cur != NULL) {
        if (cur->type == TOKEN_MAPPING) {
            Debug(0, "Function mapping found.\n");
            return 1;
        }
        cur = cur->next;
    }

    return 0;
}


static int containsAssignment(Token *head) {
    Token *cur = head;
    while (cur != NULL) {
        if (cur->type == TOKEN_ASSIGNMENT) return 1;
        cur = cur->next;
    }

    return 0;
}


static PARSER_RESULT parseFunctionCalls(Token **head) {
    Token *cur = *head;
    Token *funcPrev = NULL;

    while (cur != NULL) {
        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL_PLACEHOLDER) {
            Token *callPlaceholder = cur;
            Token *prev = NULL;

            Token *callToken = NULL;
            FunctionCall *call = NULL;

            // Prepares list of parameters
            int size = cur->nParams;
			Debug(0, "%d parameters\n", size);
            int nParameters = 0;
            Expression **paramExprs = malloc(sizeof(Expression *) * size);
            if (paramExprs == NULL) goto error;

            // Selects opening paren to be freed
            Token *seperator = cur->next;

            // skips to start of parameter
            cur = seperator->next;

            callPlaceholder->next = NULL;

            // Loops through function call
            while (cur != NULL && cur->type != TOKEN_RIGHT_PAREN) {
                Token *paramHead = cur;
                int depth = 0;

				free(seperator->value);
				free(seperator);

				if (nParameters + 1 > size) {
					printf("Invalid number of parameters for function %s\n", callPlaceholder->value);	
					goto parameter_syntax_error;
				}

                // Loops until end of parameter
                while (!(depth == 0 && cur->type == TOKEN_SEPARATOR) && !(depth == 0 && cur->type == TOKEN_RIGHT_PAREN)) {
                    if (cur->type == TOKEN_LEFT_PAREN) depth ++;
                    if (cur->type == TOKEN_RIGHT_PAREN) depth --;
                    
                    prev = cur;
                    cur = cur->next;
                }

                // Cur is now at either ',' or ')', so prev is last token in parameter
                prev->next = NULL;
                seperator = cur;
				cur = cur->next;


                // Recursively parses calls
				PARSER_RESULT callOut = parseFunctionCalls(&paramHead);
                if (callOut.type != PARSER_SUCCESS) {
					if (callOut.type == PARSER_ERROR) goto parameter_error;
					goto parameter_syntax_error;
				}

                // ===========================================================
                // Look into ways to reduce recursive calls to these functions
                // ===========================================================

                // Generates ast for parameter
                RPNList *rpn = shuntingYard(paramHead);
                if (rpn == NULL) goto parameter_error;
                
                Expression *expr = expressionFromRPN(rpn);
                free(rpn->items);
                free(rpn);
                if (expr == NULL) goto parameter_error;
                
                // ==================
                // Simplify parameter
                // ==================

				Debug(0, "Parameter expr\n");
				Debug(1, printExpression(expr));

                paramExprs[nParameters] = expr;
                nParameters ++;

                freeTokens(paramHead);

                continue;
				
				parameter_syntax_error:
                    if (rpn != NULL) free(rpn->items);
                    free(rpn);

					goto syntax_error;

                parameter_error:
                    if (rpn != NULL) free(rpn->items);
                    free(rpn);
                    
                    goto error;
            }
           

            // Creates new function call token
            Debug(0, "Creating new function call token.\n");
            callToken = calloc(1, sizeof(Token));
            if (callToken == NULL) goto error;
            
            callToken->type = TOKEN_FUNC_CALL;
            callToken->next = seperator->next;

            call = malloc(sizeof(FunctionCall));
            if (call == NULL) goto error;
            
            call->identifier = strdup(callPlaceholder->value);
            if (call->identifier == NULL) goto error;
            call->nParams = nParameters;
            call->parameters = paramExprs;

            callToken->call = call;


            Debug(0, "Replacing old call token with new one.\n");

            if (funcPrev != NULL) funcPrev->next = callToken;
            else *head = callToken;


            Debug(0, "Freeing old tokens.\n");

			free(callPlaceholder->value);
            free(callPlaceholder);
			
			free(seperator->value);
			free(seperator);


            Debug(0, "Finished with handling call to: %s.\n", call->identifier);
            continue; 
			
			syntax_error:
                freeTokens(*head);
                
                for (int i = 0; i < nParameters; i ++) {
                    freeExpression(paramExprs[i]);
                }
                free(paramExprs);
                freeTokens(cur);
                freeTokens(callToken);
                if (call != NULL) free(call->identifier);
                free(call);

				return (PARSER_RESULT) {PARSER_SYNTAX_ERROR, NULL};

            error:
                perror("Error in parsing function call");
                freeTokens(*head);
                
                for (int i = 0; i < nParameters; i ++) {
                    freeExpression(paramExprs[i]);
                }
                free(paramExprs);
                freeTokens(cur);
                freeTokens(callToken);
                if (call != NULL) free(call->identifier);
                free(call);
                
                return (PARSER_RESULT) {PARSER_ERROR, NULL};
        }

        if (cur != NULL) {
            funcPrev = cur;
            cur = cur->next;
        }
    }

    Debug(0, "Finished parsing all calls, returning.\n");
	Debug(1, printTokens(*head));
    return (PARSER_RESULT) {PARSER_SUCCESS, NULL};
}


static PARSER_RESULT parseFunctionAssignment(Token *head) {
    char *identifier = NULL;
    RPNList *rpn = NULL;
    Expression *expr = NULL;
    Function *function = NULL;

    Debug(0, "Parsing function definition.\n");
    FunctionComponent component = IDENTIFIER;

    int size = DEFAULT_PARAMETERS_SIZE;
    char **parameters = malloc(sizeof(char *) * size);
    if (parameters == NULL) goto error;
    int nParameters = 0;
    
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
                identifier = strdup(cur->value);
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
                
                parameters[nParameters] = strdup(cur->value);
                nParameters ++;
            }
        }

        cur = cur->next;
    }

	if (component != BODY) {
		printf("Invalid function definition\n");
		goto syntax_error;
	}

    // Checks body for second assignment/mapping token
    if (containsAssignment(asgn->next) || containsFunctionAssignment(asgn->next)) {
        printf("Cannot have assignment within a function definition.\n");
        goto syntax_error;
    }

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
	NORMALIZER_RESULT normOut = handleLocalVariables(&asgn, parameters, nParameters);
	if (normOut != NORM_SUCCESS) {
		if (normOut == NORM_ERROR) goto error;
		goto syntax_error;
	}

	normOut = normalize(&asgn->next);
	if (normOut != NORM_SUCCESS) {
		if (normOut == NORM_ERROR) goto error;
		goto syntax_error;
	}

	PARSER_RESULT callOut = parseFunctionCalls(&asgn->next);
	if (callOut.type != PARSER_SUCCESS) {
		if (callOut.type == PARSER_ERROR) goto error;
		goto syntax_error;
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
    free(identifier);

	Debug(0, "Successfully parsed function assignment\n");

    return (PARSER_RESULT) {PARSER_SUCCESS, NULL};

	
	syntax_error:
        freeTokens(head);
        free(identifier);
        if (parameters != NULL) {
            for (int i = 0; i < nParameters; i ++) free(parameters[i]);
        }
        free(parameters);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        if (expr != NULL) freeExpression(expr);
        free(function);
		return (PARSER_RESULT) {PARSER_SYNTAX_ERROR, NULL};

    
    error:
        freeTokens(head);
        free(identifier);
        if (parameters != NULL) {
            for (int i = 0; i < nParameters; i ++) free(parameters[i]);
        }
        free(parameters);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        if (expr != NULL) freeExpression(expr);
        free(function);

        return (PARSER_RESULT) {PARSER_ERROR, NULL};
}


static PARSER_RESULT parseAssignment(Token *head) {
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

	PARSER_RESULT callOut = parseFunctionCalls(&asgn->next);
	if (callOut.type != PARSER_SUCCESS) {
		if (callOut.type == PARSER_ERROR) goto error;
		goto syntax_error;
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

    return (PARSER_RESULT) {PARSER_SUCCESS, NULL};
	

	syntax_error:
        free(identifier);
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeExpression(expr);

		return (PARSER_RESULT) {PARSER_SYNTAX_ERROR, NULL};


    error:
        free(identifier);
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeExpression(expr);
        
        return (PARSER_RESULT) {PARSER_ERROR, NULL};
}


PARSER_RESULT parse(char *buffer) {
    Info(0, "\nParsing: '%s'\n", buffer);
    
    PARSER_RESULT result = {PARSER_ERROR, NULL};
    
    Token *head = NULL;
    RPNList *rpn = NULL;
    Expression *expr = NULL;
    
    head = tokenize(buffer);
    if (head == NULL) return result;

	NORMALIZER_RESULT normOut = normalize(&head);
    if (normOut == NORM_ERROR) goto error;
	else if (normOut == NORM_SYNTAX_ERROR) goto syntax_error;

	Debug(0, "Normalizer out\n");
	Debug(1, printTokens(head));

    if (containsAssignment(head)) {
		PARSER_RESULT asgnOut;
        if (containsFunctionAssignment(head)) {
            return parseFunctionAssignment(head);
        }
        return parseAssignment(head);
    }

	PARSER_RESULT callOut = parseFunctionCalls(&head);
	if (callOut.type != PARSER_SUCCESS) {
		if (callOut.type == PARSER_ERROR) goto error;
		goto syntax_error;
	}

    rpn = shuntingYard(head);
    if (rpn == NULL) goto error;

    expr = expressionFromRPN(rpn);

    free(rpn->items);
    free(rpn);
    freeTokens(head);

    return (PARSER_RESULT) {PARSER_SUCCESS, expr};

	syntax_error:
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
		return (PARSER_RESULT) {PARSER_SYNTAX_ERROR, expr};

    error:
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        return result;
}
