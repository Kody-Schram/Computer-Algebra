#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "core/utils/log.h"

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


static int containsFunctionAssignment(Token *head) {
    Token *cur = head;
    while (cur != nullptr) {
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
    while (cur != nullptr) {
        if (cur->type == TOKEN_ASSIGNMENT) return 1;
        cur = cur->next;
    }

    return 0;
}


static int parseFunctionCalls(Token **head) {
    Token *cur = *head;
    Token *funcPrev = nullptr;

    while (cur != nullptr) {
        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL_PLACEHOLDER) {
            Debug(0, "Function call: %s found\n", cur->value);
            Token *funcCall = cur;
            Token *prev = nullptr;

            Token *callToken = nullptr;
            FunctionCall *call = nullptr;

            // Prepares list of parameters
            int size = DEFAULT_PARAMETERS_SIZE;
            int nParameters = 0;
            ASTNode **paramASTs = malloc(sizeof(ASTNode *) * size);
            if (paramASTs == nullptr) goto error;

            // Selects opening paren to be freed
            Token *opening = cur->next;
            Token *seperator = nullptr;

            // skips to start of parameter
            cur = cur->next->next;

            // Free opening parenthesis
            free(opening->value);
            free(opening);

            funcCall->next = nullptr;

            // Loops through function call
            while (cur != nullptr && cur->type != TOKEN_RIGHT_PAREN) {
                Token *paramHead = cur;
                int parens = 0;

                if (seperator != nullptr) {
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
                prev->next = nullptr;
                seperator = cur;
                cur = cur->next;

                // Reallocates parameters list if needed
                if (nParameters >= size - 1) {
                    size += DEFAULT_PARAMETERS_SIZE; 
                    ASTNode **temp = realloc(paramASTs, sizeof(ASTNode *) * size);

                    if (temp == nullptr) goto parameter_error;

                    paramASTs = temp;

                }

                // Recursively parses calls
                if (!parseFunctionCalls(&paramHead)) goto parameter_error;

                // Generates ast for parameter
                RPNList *rpn = shuntingYard(paramHead);
                if (rpn == nullptr) goto parameter_error;
                
                ASTNode *ast = astFromRPN(rpn);
                free(rpn->items);
                free(rpn);
                if (ast == nullptr) goto parameter_error;

                paramASTs[nParameters] = ast;
                nParameters ++;

                freeTokens(paramHead);

                continue;

                parameter_error:
                    if (rpn != nullptr) free(rpn->items);
                    free(rpn);
                    
                    goto error;
            }
            
            // Creates new function call token
            Debug(0, "Creating new function call token.\n");
            callToken = calloc(1, sizeof(Token));
            if (callToken == nullptr) goto error;
            
            callToken->type = TOKEN_FUNC_CALL;
            callToken->next = seperator->next;

            call = malloc(sizeof(FunctionCall));
            if (call == nullptr) goto error;
            
            call->identifier = strdup(funcCall->value);
            if (call->identifier == nullptr) goto error;
            call->nParams = nParameters;
            call->parameters = paramASTs;

            callToken->call = call;


            Debug(0, "Replacing old call token with new one.\n");

            if (funcPrev != nullptr) funcPrev->next = callToken;
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
                    freeAST(paramASTs[i]);
                }
                free(paramASTs);
                freeTokens(cur);
                freeTokens(callToken);
                if (call != nullptr) free(call->identifier);
                free(call);
                
                return 0;
        }

        if (cur != nullptr) {
            funcPrev = cur;
            cur = cur->next;
        }
    }

    Debug(0, "Finished parsing all calls, returning.\n");
    return 1;
}


static ASTNode *parseFunctionAssignment(Token *head) {
    ASTNode *assignment = dummyASTNode(NODE_ASSIGN_FUNC);
    if (assignment == nullptr) goto error;

    ASTNode *identifier = dummyASTNode(NODE_VARIABLE);
    if (identifier == nullptr) goto error;

    assignment->func = nullptr;
    identifier->identifier = nullptr;

    RPNList *rpn = nullptr;
    ASTNode *ast = nullptr;
    Function *function = nullptr;

    Debug(0, "Parsing function definition.\n");
    FunctionComponent component = IDENTIFIER;

    Environment *localEnv = createEnvironment();
    if (localEnv == nullptr) goto error;
    Token *cur = head;
    Token *asgn = nullptr;
    while (cur != nullptr && component != BODY) {
        if (component == IDENTIFIER) {
            // Switches to PARAMETERS if ':' is found
            if (cur->type == TOKEN_ASSIGNMENT) {
                Debug(0, "Moving to function parameters.\n");
                component = PARAMETERS;
            } else if (cur->type != TOKEN_IDENTIFIER) {
                printf("Invalid token: '%s' within function identifier.\n", cur->value);
                goto error;
            } else {
                if (identifier->identifier != nullptr) {
                    printf("Invalid function declaration.\n");
                }
                identifier->identifier = strdup(cur->value);
                if (identifier->identifier == nullptr) goto error;
            }
        } else if (component == PARAMETERS) {
            // Switches to BODY if '->' is found
            if (cur->type == TOKEN_MAPPING) {
                Debug(0, "Moving to function body.\n");
                if (localEnv->entries == 0) {
                    printf("No parameters were passed. If this is intentional, define a variable instead.\n");
                    goto error;
                }
                asgn = cur;
                component = BODY;
            } else if (cur->type != TOKEN_IDENTIFIER && cur->type != TOKEN_SEPARATOR) {
                printf("Invalid token: '%s' within function parameters.\n", cur->value);
                goto error;
            } else if (cur->type == TOKEN_IDENTIFIER) {
                Debug(0, "Binding parameter '%s' to local environment.\n", cur->value);
                ASTNode *dummy = dummyASTNode(NODE_DOUBLE);
                if (dummy == nullptr) {
                    perror("Error in parsing function definition");
                    goto error;
                }

                dummy->value = 0;

                if (!bindComponent(localEnv, VARIABLE, cur->value, dummy)) {
                    free(dummy);
                    goto error;
                }
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
    if (!handleLocalVariables(&asgn, localEnv)) goto error;
    if (!lex(&asgn->next)) goto error;

    if (!parseFunctionCalls(&asgn->next)) goto error;

    rpn = shuntingYard(asgn->next);
    if (rpn == nullptr) goto error;

    // Generate ast for body
    ast = astFromRPN(rpn);
    if (ast == nullptr) goto error;

    Debug(1, printAST(ast));

    // Add to function table
    function = malloc(sizeof(Function));
    if (function == nullptr) goto error;

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
        if (identifier != nullptr) free(identifier->identifier);
        free(identifier);
        freeEnvironment(localEnv);
        if (rpn != nullptr) free(rpn->items);
        free(rpn);

        return nullptr;
}


static ASTNode *parseAssignment(Token *head) {
    ASTNode *assignment = dummyASTNode(NODE_ASSIGN_VAR);
    if (assignment == nullptr) return nullptr;

    ASTNode *identifer = dummyASTNode(NODE_VARIABLE);
    if (identifer == nullptr) goto error;

    RPNList *rpn = nullptr;
    ASTNode *ast = nullptr;

    Token *cur = head;
    Token *asgn = nullptr;
    while (cur != nullptr && asgn == nullptr) {
        if (identifer->identifier == nullptr) {
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
    if (rpn == nullptr) goto error;

    // Generate ast for body
    ast = astFromRPN(rpn);
    if (ast == nullptr) goto error;

    assignment->left = identifer;
    assignment->right = ast;

    freeTokens(head);
    free(rpn->items);
    free(rpn);

    return assignment;

    error:
        free(assignment);
        if (identifer != nullptr) free(identifer->identifier);
        free(identifer);
        freeTokens(head);
        if (rpn != nullptr) free(rpn->items);
        free(rpn);
        freeAST(ast);
        
    return nullptr;
}


ASTNode *parse(char *buffer) {
    Info(0, "\nParsing: '%s'\n", buffer);
    Token *head = nullptr;
    ASTNode *ast = nullptr;
    
    head = tokenize(buffer);
    if (head == nullptr) return nullptr;

    if (!lex(&head)) {
        freeTokens(head);
        return nullptr;
    }

    if (containsAssignment(head)) {
        if (!containsFunctionAssignment(head)) return parseAssignment(head);

        return parseFunctionAssignment(head);
    }

    if (!parseFunctionCalls(&head)) {
        freeTokens(head);
        return nullptr;
    }

    RPNList *RPN = shuntingYard(head);
    if (RPN == nullptr) {
        freeTokens(head);
        return nullptr;
    }

    ast = astFromRPN(RPN);

    free(RPN->items);
    free(RPN);
    freeTokens(head);

    return ast;
}