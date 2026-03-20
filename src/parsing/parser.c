#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "parsing/codegen/tokenizer.h"
#include "parsing/codegen/lexer.h"
#include "parsing/codegen/ast.h"
#include "utils/env/environment.h"
#include "parserUtils.h"

static const int DEFAULT_PARAMETERS_SIZE = 3;

typedef enum FunctionComponent {
    IDENTIFIER,
    PARAMETERS,
    BODY
} FunctionComponent;


void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        printf("freeing %s\n", current->value);
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
    printf("done freeing tokens\n");
}


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


static int parseFunctionCalls(Token **head, Config *config) {
    Token *cur = *head;
    Token *funcPrev = NULL;

    while (cur != NULL) {
        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Current: %s\n", cur->value);

        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL) {
            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "function found\n");
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
                    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "freed seperator\n");
                }


                while(!(parens == 0 && cur->type == TOKEN_SEPERATOR) && !(parens == 0 && cur->type == TOKEN_RIGHT_PAREN)) {
                    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nparameter token %s\n", cur->value);
                    
                    if (cur->type == TOKEN_LEFT_PAREN) parens ++;

                    if (cur->type == TOKEN_RIGHT_PAREN) parens --;
                    
                    prev = cur;
                    cur = cur->next;
                }

                if (config->LOG_LEVEL >= DEBUG) {
                    fprintf(config->LOG_STREAM, "out of loop\n");
                    fprintf(config->LOG_STREAM, "seperator token %s\n", cur->value);
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
                    printTokens(paramHead, config->LOG_STREAM);

                    fprintf(config->LOG_STREAM, "Handling recursive calls\n");
                }
                // Recursively parses calls
                if (!parseFunctionCalls(&paramHead, config) && config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "No recursive function calls found\n");
                else if(config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Recursive function calls handled\n");

                // Generates ast for parameter
                RPNList *rpn = shuntingYard(paramHead);
                if (rpn == NULL) return 1;
                ASTNode *ast = astFromRPN(rpn);
                if (ast == NULL) return 1;

                if (config->LOG_LEVEL >= DEBUG) {
                    printf("\nParameter AST\n");
                    printAST(ast, config->LOG_STREAM);
                }

                paramASTs[nParameters] = ast;
                nParameters ++;

                if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "freeing parameter tokens\n");

                // Cleanup
                freeTokens(paramHead);
            }

            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "finishing call\n");

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

            if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "finished will handling call\n");
            continue;
        }

        funcPrev = cur;
        cur = cur->next;
    }

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Finished parsing all calls, returning\n");

    return 0;
}


static ASTNode *parseFunctionDefinition(Token *head, Environment *env, Config *config) {
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

                printf("Adding parameter: %s\n", cur->value);
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

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Second round of parsing.\n");

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nLexing Tokens\n");

    // Redoes identifier tokens now that local variables for parameters are established, then redoes lexing
    handleLocalVariables(&cur, env, nParams, parameters);
    cur = lex(cur);
    if (config->LOG_LEVEL >= DEBUG) {
        fprintf(config->LOG_STREAM, "\nFunction Lexing\n");
        printTokens(cur, config->LOG_STREAM);
    }

    RPNList *rpn = shuntingYard(cur);
    if (rpn == NULL) return NULL;

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nGenerating AST.\n");
    // Generate ast for body
    ASTNode *ast = astFromRPN(rpn);
    if (ast == NULL) return NULL;

    if (config->LOG_LEVEL >= DEBUG) printAST(ast, config->LOG_STREAM);

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

    freeTokens(head);

    return assignment;
}


static ASTNode *parseAssignment(Token *head) {
    //ASTNode *identifier = (ASTNode*) head->value;
    printf("%s", head->value);
    return NULL;
}


ASTNode *parse(char *buffer, Environment *env, Config *config) {
    if (config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "\nParsing\n");
    
    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nTokenizing\n");
    Token *raw = tokenize(buffer, env);
    if (raw == NULL) return NULL;
    if (config->LOG_LEVEL >= DEBUG) printTokens(raw, config->LOG_STREAM);

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nLexing Tokens\n");
    Token *head = lex(raw);
    if (head == NULL) return NULL;
    if (config->LOG_LEVEL >= DEBUG) printTokens(head, config->LOG_STREAM);

    if (containsAssignment(head)) {
        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Contains assignment.\n");
        if (!containsFunctionDefinition(head)) return parseAssignment(head);

        return parseFunctionDefinition(head, env, config);
    }

    if (parseFunctionCalls(&head, config)) {
        fprintf(config->LOG_STREAM, "Error parsing function call(s)\n");
        return NULL;
    }

    if (config->LOG_LEVEL >= DEBUG) {
        fprintf(config->LOG_STREAM, "\nPost Function Call Tokens\n");
        printTokens(head, config->LOG_STREAM);
    }

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nCreating RPN\n");
    RPNList *RPN = shuntingYard(head);
    if (RPN == NULL) return NULL;
    if (config->LOG_LEVEL >= DEBUG) printRPN(*RPN, config->LOG_STREAM);

    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "\nGenerating AST.\n");
    ASTNode *ast = astFromRPN(RPN);

    if (config->LOG_LEVEL >= DEBUG && ast != NULL) {
        fprintf(config->LOG_STREAM, "\nPrinting AST\n");
        printAST(ast, config->LOG_STREAM);
    }

    if (config->LOG_LEVEL >= INFO) fprintf(config->LOG_STREAM, "\nInput Parsed\n");

    freeTokens(head);
    return ast;
}