#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "codegen/tokenizer.h"
#include "codegen/lexer.h"
#include "codegen/ast.h"
#include "tables/functions.h"

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

void printRPN(RPNList list) {
    printf("RPN: ");

    for (int i = 0; i < list.length; i ++) {
        printf("%s ", list.items[i]->value);
    }

    printf("\n");
}

int containsFunctionDefinition(Token *head) {
    Token *cur = head;
    while (cur != NULL) {
        if (cur->type == TOKEN_FUNC_DEF) return 1;
        cur = cur->next;
    }

    return 0;
}

int parseFunctionDefinition(Token *head) {
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
            ASTNode *head = astFromRPN(rpn);
            if (head == NULL) return 0;

            // Add to function table
            Function *function = malloc(sizeof(function));
            if (function == NULL) return 0;

            function->identifier = identifier;
            function->nParameters = nParams;
            function->parameters = parameters;
            function->type = DEFINED;
            function->definition = head;

            return addFunction(function);

        }

        cur = cur->next;
    }
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

ASTNode *parse(char *buffer, int withinFunction) {
    printf("\nTokenizing Input\n");
    Token *raw = tokenize(buffer);
    if (raw == NULL) return NULL;
    printf("Success\n");

    printf("\nLexing Tokens\n");
    Token *head = lex(raw);
    if (head == NULL) return NULL;
    printf("Success\n");

    // Parses function definitions differently than regular expressions
    if (containsFunctionDefinition(head)) {
        if (withinFunction) {
            printf("Cannot have nested functions definitions.\n");
            return NULL;
        }
        parseFunctionDefinition(head);
    } else {
        printf("\nCreating RPN\n");
        RPNList *RPN = shuntingYard(head);
        if (RPN == NULL) return NULL;
        printRPN(*RPN);

        printf("\nGenerating AST.\n");
        ASTNode *ast = astFromRPN(RPN);

        printf("\nPrinting AST\n");
        printAST(ast);

        freeTokens(head);
        return ast;
    }

    return NULL;
}