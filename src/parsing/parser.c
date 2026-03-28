#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "utils/context/context.h"
#include "utils/log.h"

#include "parsing/codegen/tokenizer.h"
#include "parsing/codegen/lexer.h"
#include "parsing/codegen/ast.h"
#include "parserUtils.h"

static const int DEFAULT_PARAMETERS_SIZE = 3;

typedef enum FunctionComponent {
    IDENTIFIER,
    PARAMETERS,
    BODY
} FunctionComponent;

static int containsFunctionDefinition(Token *head) {
    Token *cur = head;
    while (cur != NULL) {
        if (cur->type == TOKEN_MAPPING) {
            Debug(0, "Assignment found.\n");
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
        if (cur->type == TOKEN_FUNC_CALL) {
            Debug(0, "Function call: %s found\n", cur->value);
            Token *funcCall = cur;
            Token *prev = NULL;

            Token *callToken = NULL;
            FunctionCall *call = NULL;

            // Prepares list of parameters
            int size = DEFAULT_PARAMETERS_SIZE;
            int nParameters = 0;
            ASTNode **paramASTs = malloc(sizeof(ASTNode *) * size);
            if (paramASTs == NULL) {
                printf("Error allocating for parameter ASTs.\n");
                goto error;
            }

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
                    ASTNode **temp = realloc(paramASTs, sizeof(ASTNode *) * size);

                    if (temp == NULL) goto parameter_error;

                    paramASTs = temp;

                }

                // Recursively parses calls
                if (!parseFunctionCalls(&paramHead)) goto parameter_error;

                // Generates ast for parameter
                RPNList *rpn = shuntingYard(paramHead);
                if (rpn == NULL) goto parameter_error;
                
                ASTNode *ast = astFromRPN(rpn);
                free(rpn->items);
                free(rpn);
                if (ast == NULL) goto parameter_error;

                paramASTs[nParameters] = ast;
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
            if (callToken == NULL) {
                printf("Error allocating memory for new function call token.\n");
                goto error;
            }
            callToken->type = TOKEN_FUNC_CALL;
            callToken->next = seperator->next;

            call = malloc(sizeof(FunctionCall));
            if (call == NULL) {
                printf("Error allocating memory for function call.\n");
                goto error;
            }
            call->identifier = strdup(funcCall->value);
            if (call->identifier == NULL) goto error;
            call->nParams = nParameters;
            call->parameters = paramASTs;

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
                freeTokens(*head);
                
                for (int i = 0; i < nParameters; i ++) {
                    freeAST(paramASTs[i]);
                }
                free(paramASTs);
                freeTokens(cur);
                freeTokens(callToken);
                if (call != NULL) free(call->identifier);
                free(call);
                
                return 0;
        }

        Debug(0, "Finished parsing a function call.\n");

        if (cur != NULL) {
            funcPrev = cur;
            cur = cur->next;
        }
    }

    Debug(0, "Finished parsing all calls, returning.\n");
    return 1;
}


static ASTNode *parseFunctionDefinition(Token *head) {
    ASTNode *assignment = dummyASTNode(NODE_ASSIGN_FUNC);
    if (assignment == NULL) goto error;

    ASTNode *identifier = dummyASTNode(NODE_VARIABLE);
    if (identifier == NULL) goto error;

    assignment->func = NULL;
    identifier->identifier = NULL;

    RPNList *rpn = NULL;
    ASTNode *ast = NULL;
    Function *function = NULL;

    Debug(0, "Parsing function definition.\n");
    FunctionComponent component = IDENTIFIER;

    Environment *localEnv = createEnvironment();
    if (localEnv == NULL) goto error;
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
                if (identifier->identifier != NULL) {
                    printf("Invalid function declaration.\n");
                }
                identifier->identifier = strdup(cur->value);
                if (identifier->identifier == NULL) goto error;
            }
        } else if (component == PARAMETERS) {
            // Switches to BODY if '->' is found
            if (cur->type == TOKEN_MAPPING) {
                Debug(0, "Moving to function body.\n");
                asgn = cur;
                component = BODY;
            } else if (cur->type != TOKEN_IDENTIFIER && cur->type != TOKEN_SEPARATOR) {
                printf("Invalid token: '%s' within function parameters.\n", cur->value);
                goto error;
            } else if (cur->type == TOKEN_IDENTIFIER) {
                Debug(0, "Binding parameter '%s' to local environment.\n", cur->value);
                ASTNode *dummy = dummyASTNode(NODE_DOUBLE);
                dummy->value = 0;

                if (!bindComponent(localEnv, VARIABLE, cur->value, dummy)) {
                    free(dummy);
                    goto error;
                }
            }
        }

        cur = cur->next;
    }

    if (containsAssignment(asgn->next)) {
        printf("Cannot have assignment within a function definition.\n");
        goto error;
    }

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
    handleLocalVariables(&asgn, localEnv);
    if (!lex(&asgn->next)) goto error;

    if (!parseFunctionCalls(&asgn->next)) {
        printf("Error parsing function call(s)\n");
        goto error;
    }

    rpn = shuntingYard(asgn->next);
    if (rpn == NULL) goto error;

    // Generate ast for body
    ast = astFromRPN(rpn);
    if (ast == NULL) goto error;

    Debug(1, printAST(ast));

    // Add to function table
    function = malloc(sizeof(Function));
    if (function == NULL) goto error;

    function->env = localEnv;
    function->type = DEFINED;
    function->definition = ast;

    Debug(1, printEnvironment(localEnv));

    assignment->func = function;
    assignment->left = identifier;

    freeTokens(head);
    free(rpn->items);
    free(rpn);

    return assignment;

    error:
        freeTokens(head);
        free(assignment);
        if (identifier != NULL) free(identifier->identifier);
        free(identifier);
        freeEnvironment(localEnv);
        if (rpn != NULL) free(rpn->items);
        free(rpn);

        return NULL;
}


static ASTNode *parseAssignment(Token *head) {
    ASTNode *assignment = dummyASTNode(NODE_ASSIGN_VAR);
    if (assignment == NULL) return NULL;

    ASTNode *identifer = dummyASTNode(NODE_VARIABLE);
    if (identifer == NULL) goto error;

    RPNList *rpn = NULL;
    ASTNode *ast = NULL;

    Token *cur = head;
    Token *asgn = NULL;
    while (cur != NULL && asgn == NULL) {
        if (identifer->identifier == NULL) {
            if (cur->type == TOKEN_IDENTIFIER) identifer->identifier = strdup(cur->value);
            else {
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
    ast = astFromRPN(rpn);
    if (ast == NULL) goto error;

    assignment->left = identifer;
    assignment->right = ast;

    freeTokens(head);
    free(rpn->items);
    free(rpn);

    return assignment;

    error:
        free(assignment);
        if (identifer != NULL) free(identifer->identifier);
        free(identifer);
        freeTokens(head);
        if (rpn != NULL) free(rpn->items);
        free(rpn);
        freeAST(ast);
}


ASTNode *parse(char *buffer) {
    Info(0, "\nParsing: '%s'\n", buffer);
    Token *head = NULL;
    ASTNode *ast = NULL;
    
    head = tokenize(buffer);
    if (head == NULL) return NULL;

    if (!lex(&head)) return NULL;

    if (containsAssignment(head)) {
        if (!containsFunctionDefinition(head)) return parseAssignment(head);

        return parseFunctionDefinition(head);
    }

    if (!parseFunctionCalls(&head)) {
        printf("Error parsing function call(s)\n");
        freeTokens(head);
        return NULL;
    }

    RPNList *RPN = shuntingYard(head);
    if (RPN == NULL) {
        freeTokens(head);
        return NULL;
    }

    ast = astFromRPN(RPN);

    free(RPN->items);
    free(RPN);

    freeTokens(head);

    return ast;
}