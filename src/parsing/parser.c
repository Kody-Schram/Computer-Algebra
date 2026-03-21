#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "utils/context/context.h"

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
        if (cur->type == TOKEN_FUNC_DEF) return 1;
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
    Config *config = GLOBALCONTEXT->config;

    Token *cur = *head;
    Token *funcPrev = NULL;

    while (cur != NULL) {
        //if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Current: %s\n", cur->value);

        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL) {
            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Function call: %s found\n", cur->value);
            Token *funcCall = cur;
            Token *prev = NULL;

            // Prepares list of parameters
            int size = DEFAULT_PARAMETERS_SIZE;
            int nParameters = 0;
            ASTNode **paramASTs = malloc(sizeof(ASTNode *) * size);
            if (paramASTs == NULL) {
                printf("Error allocating for parameter ASTs.\n");
                return 1;
            }

            // Selects opening paren to be freed
            Token *opening = cur->next;
            Token *seperator = NULL;

            // skips to start of parameter
            cur = cur->next->next;
            prev = cur;

            while (cur != NULL && cur->type != TOKEN_RIGHT_PAREN) {
                Token *paramHead = cur;
                int parens = 0;

                if (seperator != NULL) {
                    free(seperator->value);
                    free(seperator);
                }


                while(!(parens == 0 && cur->type == TOKEN_SEPERATOR) && !(parens == 0 && cur->type == TOKEN_RIGHT_PAREN)) {
                    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Handling parameter token: %s.\n", cur->value);
                    
                    if (cur->type == TOKEN_LEFT_PAREN) parens ++;

                    if (cur->type == TOKEN_RIGHT_PAREN) parens --;
                    
                    prev = cur;
                    cur = cur->next;
                }

                // Cur is now at either ',' or ')', so prev is last token in parameter
                prev->next = NULL;
                seperator = cur;
                cur = cur->next;

                if (nParameters >= size - 1) {
                    size += DEFAULT_PARAMETERS_SIZE; 
                    ASTNode **temp = realloc(paramASTs, sizeof(ASTNode *) * size);

                    if (temp == NULL) {
                        printf("Error reallocating for more function call parameters.\n");
                        return 1;
                    }

                    paramASTs = temp;

                }

                if (config->LOG_LEVEL >= DEBUG) {
                    fprintf(config->LOG_STREAM, "\nParameter Tokens\n");
                    printTokens(paramHead);

                    fprintf(config->LOG_STREAM, "Handling recursive calls\n");
                }
                // Recursively parses calls
                if (!parseFunctionCalls(&paramHead) && config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "No recursive function calls found\n");
                else if(config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Recursive function calls handled\n");

                // Generates ast for parameter
                RPNList *rpn = shuntingYard(paramHead);
                if (rpn == NULL) return 1;
                
                ASTNode *ast = astFromRPN(rpn);
                if (ast == NULL) return 1;

                if (config->LOG_LEVEL >= DEBUG) {
                    fprintf(config->LOG_STREAM, "\nParameter AST\n");
                    printAST(ast);
                }

                paramASTs[nParameters] = ast;
                nParameters ++;

                if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Freeing parameter tokens.\n");

                // Cleanup
                freeTokens(paramHead);
            }
            
            // Creates new function call token
            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Creating new function call token.\n");
            Token *callToken = malloc(sizeof(Token));
            if (callToken == NULL) {
                printf("Couldn't allocate memory for new function call token.\n");
                return 1;
            }
            callToken->type = TOKEN_FUNC_CALL;
            callToken->next = seperator->next;

            FunctionCall *call = malloc(sizeof(FunctionCall));
            call->identifier = strdup(funcCall->value);
            call->nParams = nParameters;
            call->parameters = paramASTs;

            callToken->call = call;


            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Replacing old call token with new one.\n");

            if (funcPrev != NULL) funcPrev->next = callToken;
            else *head = callToken;

            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Freeing old tokens.\n");
            // Free opening parenthesis
            free(opening->value);
            free(opening);

            free(seperator->value);
            free(seperator);
        
            // Replaces original funciton call token
            free(funcCall->value);
            free(funcCall);

            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Finished with handling call to: %s.\n", call->identifier);
            continue;
        }

        funcPrev = cur;
        cur = cur->next;
    }

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Finished parsing all calls, returning.\n");

    return 0;
}


static ASTNode *parseFunctionDefinition(Token *head) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Parsing function definition.\n");
    FunctionComponent component = IDENTIFIER;

    char *id;

    int paramSize = DEFAULT_PARAMETERS_SIZE;
    char **parameters = malloc(paramSize * sizeof(char*));
    int nParams = 0;

    Token *cur = head;
    while (cur != NULL) {
        //printf("%s, type %d, comp %d,  == %d\n", cur->value, cur->type, component, (cur->type != TOKEN_IDENTIFIER && component == IDENTIFIER));
        
        // Checks for switching to body
        if (component == PARAMETERS && cur->type == TOKEN_ASSIGNMENT) {
            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Moving to function body.\n");
            component = BODY;
            cur = cur->next;
            break;
        } else if (cur->type == TOKEN_ASSIGNMENT) {
            printf("Invalid token: %s within component %d\n", cur->value, component);
            return NULL;
        }

        // Checks for switching to parameters
        if (component == IDENTIFIER && cur->type == TOKEN_FUNC_DEF) {
            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Moving to function parameters.\n");
            component = PARAMETERS;
            cur = cur->next;
            continue;
        } else if (cur->type == TOKEN_FUNC_DEF) {
            printf("Invalid token: %s within component %d.\n", cur->value, component);
            return NULL;
        }

        if (component == PARAMETERS && (cur->type != TOKEN_IDENTIFIER && cur->type != TOKEN_SEPERATOR)) {
            printf("Invalid token: %s within component %d.\n", cur->value, component);
            return NULL;
        }

        // Assignments identifier
        if (component == IDENTIFIER) {
            id = cur->value;
        } else {
            if (cur->type != TOKEN_SEPERATOR) {
            
                // Reallocates parameter list if needed
                if (nParams >= paramSize) {
                    paramSize *= 2;
                    char **temp = realloc(parameters, sizeof(char*) * paramSize);
                    if (temp == NULL) {
                        printf("Error reallocating more space for function parameters.\n");
                        return 0;
                    }

                    parameters = temp;
                    
                }

                if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Adding parameter: %s\n", cur->value);
                parameters[nParams] = strdup(cur->value);
                nParams ++;
            }
        }

        cur = cur->next;
    }

    if (containsAssignment(cur)) {
        printf("Cannot have assignment within a function definition.\n");
        return NULL;
    }

    // if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Second round of parsing.\n");

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nRechecking identifiers with local parameters.\n");

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
    handleLocalVariables(&cur, nParams, parameters);
    //cur = lex(cur);

    if (parseFunctionCalls(&head)) {
        fprintf(config->LOG_STREAM, "Error parsing function call(s)\n");
        return NULL;
    }

    RPNList *rpn = shuntingYard(cur);
    if (rpn == NULL) return NULL;

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nFunction Defintion AST.\n");
    // Generate ast for body
    ASTNode *ast = astFromRPN(rpn);
    if (ast == NULL) return NULL;

    if (config->LOG_LEVEL >= DEBUG) printAST(ast);

    // Add to function table
    Function *function = malloc(sizeof(Function));
    if (function == NULL) return NULL;

    function->nParameters = nParams;
    function->parameters = parameters;
    function->type = DEFINED;
    function->definition = ast;

    ASTNode *assignment = dummyASTNode(NODE_ASSIGN_FUNC);
    assignment->right = (ASTNode*) function;

    ASTNode *identifier = dummyASTNode(NODE_VARIABLE);
    identifier->identifier = strdup(id);

    assignment->left = identifier;

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Freeing tokens.\n");
    freeTokens(head);

    return assignment;
}


static ASTNode *parseAssignment(Token *head) {
    //ASTNode *identifier = (ASTNode*) head->value;
    printf("%s", head->value);
    return NULL;
}


ASTNode *parse(char *buffer) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    if (config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "\nParsing: '%s'\n", buffer);
    
    Token *raw = tokenize(buffer);
    if (raw == NULL) return NULL;
    //if (config->LOG_LEVEL >= DEBUG) printTokens(raw, config->LOG_STREAM);

    Token *head = lex(raw);
    if (head == NULL) return NULL;

    if (containsAssignment(head)) {
        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Assignment found.\n");
        if (!containsFunctionDefinition(head)) return parseAssignment(head);

        return parseFunctionDefinition(head);
    }

    if (parseFunctionCalls(&head)) {
        printf("Error parsing function call(s)\n");
        return NULL;
    }

    if (config->LOG_LEVEL >= DEBUG) {
        fprintf(config->LOG_STREAM, "\nPost Function Call Tokens\n");
        printTokens(head);
    }

    RPNList *RPN = shuntingYard(head);
    if (RPN == NULL) return NULL;

    ASTNode *ast = astFromRPN(RPN);
    if (ast == NULL) return NULL;

    if (config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "\nInput Parsed\n");

    freeTokens(head);

    return ast;
}