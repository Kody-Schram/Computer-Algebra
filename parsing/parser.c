#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "codegen/tokenizer.h"
#include "codegen/lexer.h"
#include "codegen/ast.h"
#include "env/environment.h"

const int DEFAULT_PARAMETERS_SIZE = 5;

typedef enum FunctionComponent {
    IDENTIFIER,
    PARAMETERS,
    BODY
} FunctionComponent;

void freeTokens(Token *head) {
    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}

int containsFunctionDefinition(Token *head) {
    Token *cur = head;
    while (cur != NULL) {
        if (cur->type == TOKEN_FUNC_DEF) return 1;
        cur = cur->next;
    }

    return 0;
}

int parseFunctionDefinition(Token *head, Environment *env) {
    printf("parsing function :)\n");
    FunctionComponent component = IDENTIFIER;

    char *identifier;

    int paramSize = DEFAULT_PARAMETERS_SIZE;
    char **parameters = malloc(paramSize * sizeof(char*));
    int nParams = 0;

    Token *cur = head;
    while (cur != NULL) {
        printf("%s, type %d, comp %d,  == %d\n", cur->value, cur->type, component, (cur->type != TOKEN_IDENTIFIER && component == IDENTIFIER));
        if ((cur->type != TOKEN_IDENTIFIER && component == IDENTIFIER)
            || (cur->type != TOKEN_SEPERATOR && component != PARAMETERS)) {
                printf("Invalid token within function declaration. %s %d\n", cur->value, component);
                return 0;
        }
        if (cur->type == TOKEN_SEPERATOR) {
            component = PARAMETERS;
            cur = cur->next;
            continue;
        }
        if (cur->type == TOKEN_ASSIGNMENT) {
            component = BODY;
            cur = cur->next;
            continue;
        }
        if (component == IDENTIFIER) {
            identifier = strdup(cur->value);
            component = PARAMETERS;
        } 
        else if (component == PARAMETERS) {
            // Reallocates parameter list if needed
            if (nParams >= paramSize) {
                paramSize *= 2;
                char **temp = realloc(parameters, paramSize);
                if (temp == NULL) {
                    printf("Error reallocating more space for function parameters.\n");
                    return 0;
                }

                parameters = temp;
                
            }
            parameters[nParams] = strdup(cur->value);
            nParams ++;

        // Adds remaining tokens to new body linked list
        } else if (component == BODY) {
            RPNList *rpn = shuntingYard(cur);
            if (rpn == NULL) return 0;

            // Generate ast for body
            ASTNode *head = astFromRPN(rpn, env);
            if (head == NULL) return 0;

            // Add to function table
            Function *function = malloc(sizeof(Function));
            if (function == NULL) return 0;

            function->nParameters = nParams;
            function->parameters = parameters;
            function->type = DEFINED;
            function->definition = head;

            // Adds function to the environment
            bindComponent(env, FUNCTION, identifier, function);
            return 1;

        }

        cur = cur->next;
    }

    return 0;
}

void parseFunctionCalls(Token *head) {
    Token **parameters = NULL;
    int nParameters = 0;

    Token *cur = head;
    Token *prev = NULL;

    while (cur != NULL) {

        // Recursively parses nested function calls
        if (cur->type == TOKEN_FUNC_CALL) {
            // Skips opening parens, has been handled by the lexer already
            prev = cur->next;
            cur = cur->next->next;

            Token *newFunc = malloc(sizeof(Token));

            Token *end = NULL;
            int parens = 1;
            while (parens > 0) {

            }
            end = cur;

        }

        prev = cur;
        cur = cur->next;
    }

}

ASTNode *parse(char *buffer, Environment *env, int withinFunction, int debugging) {
    if (debugging) printf("\nTokenizing Input\n");
    Token *raw = tokenize(buffer, env);
    if (raw == NULL) return NULL;

    if (debugging) printf("\nLexing Tokens\n");
    Token *head = lex(raw);
    if (head == NULL) return NULL;

    // Parses function definitions differently than regular expressions
    if (containsFunctionDefinition(head)) {
        if (withinFunction) {
            printf("Cannot have nested functions definitions.\n");
            return NULL;
        }
        parseFunctionDefinition(head, env);
    } else {
        if (debugging) printf("\nCreating RPN\n");
        RPNList *RPN = shuntingYard(head);
        if (RPN == NULL) return NULL;
        if (debugging) printRPN(*RPN);

        if (debugging) printf("\nGenerating AST.\n");
        ASTNode *ast = astFromRPN(RPN, env);

        if (debugging) {
            printf("\nPrinting AST\n");
            printAST(ast);
        }

        freeTokens(head);
        return ast;
    }

    return NULL;
}