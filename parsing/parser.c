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
    for (int i = 0; i < list.length; i ++) {
        printf("%s", list.items[i]->value);
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

    Token *bodyHead;
    Token *bodyCur;

    Token *cur = head;
    while (cur != NULL) {
        if ((cur->type != TOKEN_IDENTIFIER && component == IDENTIFIER)
            || (cur->type != TOKEN_SEPERATOR && component != PARAMETERS)) {
                printf("Invalid token within function declaration. %s %d\n", cur->value, component);
                return 0;
        }
        if (cur->type == TOKEN_SEPERATOR) {
            component = PARAMETERS;
            continue;
        }
        if (cur->type == TOKEN_ASSIGNMENT) {
            component = BODY;
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
            if (bodyHead == NULL) {
                bodyHead = cur;
                bodyCur = bodyHead;
            } else {
                bodyCur->next = cur;
                bodyCur = bodyCur->next;
            }
        }

        cur = cur->next;
    }

    //RPNList rpn = shuntingYard(bodyHead);
    // Generate ast for body

    // Add to function table

    printf("identifier: %s", identifier);
    return 1;
}

void parseFunctionCall(Token *head) {
    Token **parameters = NULL;
    int nParameters = 0;

    Token *cur = head;
    Token *prev = NULL;

    while (cur != NULL) {
        if (cur->type == TOKEN_FUNC_CALL) {

        }

        prev = cur;
        cur = cur->next;
    }

}

ASTNode *parse(char *buffer, int withinFunction) {
    printf("\nTokenizing Input\n");
    Token *raw = tokenize(buffer);
    if (raw == NULL) return NULL;

    printf("\nLexing Tokens\n");
    Token *head = lex(raw);
    
    if (head == NULL) return NULL;
    printf("continuing to final step\n");

    // Parses function definitions differently than regular expressions
    if (containsFunctionDefinition(head)) {
        if (withinFunction) {
            printf("Cannot have nested functions definitions.\n");
            return NULL;
        }
        parseFunctionDefinition(head);
    } else {
        printf("creating rpn\n");
        RPNList *RPN = shuntingYard(head);
        if (RPN == NULL) return NULL;
        printRPN(*RPN);

        printf("Generating AST.\n");
        ASTNode *ast = astFromRPN(RPN);

        freeTokens(head);
        return ast;
    }
}