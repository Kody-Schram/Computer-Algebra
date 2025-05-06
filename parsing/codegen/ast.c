#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

int getPrecedent(char *operator) {
    int precedent;
    switch(operator[0]) {
        case '+':
        case '-':
            precedent = 1;
            break;
        case '*':
        case '/':
            precedent = 2;
            break;
    }

    return precedent;
}

int getLinkedListLength(Token *head) {
    Token *cur = head;

    int i = 0;
    while (cur != NULL) {
        i ++;
        cur = cur->next;
    }

    return i;
}


RPNList shuntingYard(Token *head) {
    Token *cur = head;

    int length = getLinkedListLength(head);
    Stack output = {
        length,
        0,
        {malloc(length * sizeof(Token *))}
    };
    // Token **output = malloc(length * sizeof(Token *));
    // int outputEnd = 0;

    Stack operators = {
        length / 2,
        0,
        {malloc(length /2 * sizeof(Token *))}
    };
    // Token **operators = malloc(size * sizeof(Token *));
    // int operators.entries = 0;

    while (cur != NULL) {
        // Adds numbers, variables, functions to output stack
        if (cur->type != TOKEN_LEFT_PAREN && cur->type != TOKEN_RIGHT_PAREN) {
            if (cur->type == TOKEN_NUMBER || cur->type == TOKEN_IDENTIFIER || cur->type == TOKEN_FUNC_CALL) {
                output.tokens[output.entries] = cur;
                output.entries ++;
            } else {
                // At least one operator in, check precedence
                if (operators.entries > 0) {
                    if (getPrecedent(operators.tokens[operators.entries-1]->value) >= getPrecedent(cur->value)) {
                        // Flush operators to output stack
                        while (operators.entries > 0) {
                            output.tokens[output.entries] = operators.tokens[operators.entries-1];
                            output.entries ++;
                            operators.entries --;
                        }
                    } else {
                        if (operators.entries >= operators.length) {
                            operators.length *= 2;
                            Token **temp = realloc(operators.tokens, operators.length * sizeof(Token *));
                            if (temp == NULL) {
                                printf("Cannot allocate more space for operator stack.");
                                free(operators.tokens);
                                free(output.tokens);

                                return (RPNList) {0, NULL};
                            }
                        }

                        operators.tokens[operators.entries] = cur;
                        operators.entries ++;
                    }
                } else {
                    operators.tokens[operators.entries] = cur;
                    operators.entries ++;
                }
            }
        }

        cur = cur->next;
    }

    // Flush remaining operators to output stack
    while (operators.entries > 0) {
        output.tokens[output.entries] = operators.tokens[operators.entries-1];
        output.entries++;
        operators.entries --;
    }
    
    free(operators.tokens);

    return (RPNList) {output.entries, output.tokens};
}

ASTNode *astFromRPN(RPNList rpn) {
    
}

ASTNode *generateAST(Token *head) {
    RPNList rpn = shuntingYard(head);
    if (rpn.length == 0) return NULL;

    ASTNode *ast = astFromRPN(rpn);
    return ast;
}