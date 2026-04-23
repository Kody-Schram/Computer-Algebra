#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"

#include "core/parsing/codegen/tokenizer.h"
#include "core/parsing/codegen/lexer.h"
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


static int parseFunctionCalls(Token **head) {
    Token *cur = *head;
    Token *funcPrev = NULL;

    while (cur != NULL) {
        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL_PLACEHOLDER) {
            Debug(0, "Function call: %s found\n", cur->value);
            Token *funcCall = cur;
            Token *prev = NULL;

            Token *callToken = NULL;
            FunctionCall *call = NULL;

            // Prepares list of parameters
            int size = DEFAULT_PARAMETERS_SIZE;
            int nParameters = 0;
            Expression **paramExprs = malloc(sizeof(Expression *) * size);
            if (paramExprs == NULL) goto error;

            // Selects opening paren to be freed
            Token *opening = cur->next;
            Token *seperator = NULL;

            // skips to start of parameter
            cur = cur->next->next;

            // Free opening parenthesis
            free(opening->value);
            free(opening);

            funcCall->next = NULL;

            // Loops through function call
            while (cur != NULL && cur->type != TOKEN_RIGHT_PAREN) {
                Token *paramHead = cur;
                int parens = 0;

                if (seperator != NULL) {
                    free(seperator->value);
                    free(seperator);
                }

                // Loops until end of parameter
                while(!(parens == 0 && cur->type == TOKEN_SEPARATOR) && !(parens == 0 && cur->type == TOKEN_RIGHT_PAREN)) {
                    if (cur->type == TOKEN_LEFT_PAREN) parens ++;
                    if (cur->type == TOKEN_RIGHT_PAREN) parens --;
                    
                    prev = cur;
                    cur = cur->next;
                }

                // Cur is now at either ',' or ')', so prev is last token in parameter
                prev->next = NULL;
                seperator = cur;
                cur = cur->next;

                // Reallocates parameters list if needed
                if (nParameters >= size - 1) {
                    size += DEFAULT_PARAMETERS_SIZE; 
                    Expression **temp = realloc(paramExprs, sizeof(Expression *) * size);

                    if (temp == NULL) goto parameter_error;

                    paramExprs = temp;

                }
                
                // Recursively parses calls
                if (!parseFunctionCalls(&paramHead)) goto parameter_error;
                
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

                paramExprs[nParameters] = expr;
                nParameters ++;

                freeTokens(paramHead);

                continue;

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
            
            call->identifier = strdup(funcCall->value);
            if (call->identifier == NULL) goto error;
            call->nParams = nParameters;
            call->parameters = paramExprs;

            callToken->call = call;


            Debug(0, "Replacing old call token with new one.\n");

            if (funcPrev != NULL) funcPrev->next = callToken;
            else *head = callToken;


            Debug(0, "Freeing old tokens.\n");

            free(seperator->value);
            free(seperator);
            free(funcCall->value);
            free(funcCall);


            Debug(0, "Finished with handling call to: %s.\n", call->identifier);
            continue; 

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
                
                return 0;
        }

        if (cur != NULL) {
            funcPrev = cur;
            cur = cur->next;
        }
    }

    Debug(0, "Finished parsing all calls, returning.\n");
    return 1;
}


static int parseFunctionAssignment(Token *head) {
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
                printf("Invalid token: '%s' within function identifier.\n", cur->value);
                goto error;
            } else {
                if (identifier != NULL) {
                    printf("Invalid function declaration.\n");
                    goto error;
                }
                identifier = strdup(cur->value);
                if (identifier == NULL) {
                    perror("Error parsing function assignment");
                    goto error;
                }
            }
        } else if (component == PARAMETERS) {
            // Switches to BODY if '->' is found
            if (cur->type == TOKEN_MAPPING) {
                Debug(0, "Moving to function body.\n");
                if (parameters == 0) {
                    printf("No parameters were passed. If this is intentional, define a variable instead.\n");
                    goto error;
                }
                asgn = cur;
                component = BODY;
            } else if (cur->type != TOKEN_IDENTIFIER && cur->type != TOKEN_SEPARATOR) {
                printf("Invalid token: '%s' within function parameters.\n", cur->value);
                goto error;
            } else if (cur->type == TOKEN_IDENTIFIER) {
                Debug(0, "Adding function parameter '%s'\n", cur->value);
                if (nParameters >= size) {
                    size *= 2;
                    char **temp = realloc(parameters, size);
                    if (temp == NULL) goto error;
                    
                    parameters = temp;
                }
                
                parameters[nParameters] = strdup(cur->value);
                nParameters ++;
            }
        }

        cur = cur->next;
    }

    // Checks body for second assignment/mapping token
    if (containsAssignment(asgn->next) || containsFunctionAssignment(asgn->next)) {
        printf("Cannot have assignment within a function definition.\n");
        goto error;
    }

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
    if (!handleLocalVariables(&asgn, parameters, nParameters)) goto error;
    if (!lex(&asgn->next)) goto error;

    if (!parseFunctionCalls(&asgn->next)) goto error;

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

    return 1;
    
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

        return 0;
}


static int parseAssignment(Token *head) {
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
                    goto error;
                }
            } else {
                printf("Invalid token '%s' in variable identifier.\n", cur->value);
                goto error;
            }
        } else if (cur->type == TOKEN_ASSIGNMENT) {
            asgn = cur;
        }

        cur = cur->next;
    }

    if (!parseFunctionCalls(&asgn->next)) {
        printf("Error parsing function call(s)\n");
        goto error;
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

    return 1;

    error:
        free(identifier);
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeExpression(expr);
        
        return 0;
}


ParserResult parse(char *buffer) {
    Info(0, "\nParsing: '%s'\n", buffer);
    
    ParserResult result = {PARSER_ERROR, NULL};
    
    Token *head = NULL;
    RPNList *rpn = NULL;
    Expression *expr = NULL;
    
    head = tokenize(buffer);
    if (head == NULL) return result;

    if (!lex(&head)) goto error;

    if (containsAssignment(head)) {
        if (containsFunctionAssignment(head)) {
            if (!parseFunctionAssignment(head)) return result;
            goto assignment_success;
        }
        else if  (!parseAssignment(head)) return result;
        
        assignment_success:
        return (ParserResult) {PARSER_SUCCESS, NULL};
    }

    if (!parseFunctionCalls(&head)) goto error;

    rpn = shuntingYard(head);
    if (rpn == NULL) goto error;

    expr = expressionFromRPN(rpn);

    free(rpn->items);
    free(rpn);
    freeTokens(head);

    return (ParserResult) {PARSER_SUCCESS, expr};
    
    error:
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        return result;
}