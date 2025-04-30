#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

int getPrecedent(char operator) {
    int precedent;
    switch(operator) {
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

    Token **output = malloc(length * sizeof(Token *));
    int outputEnd = 0;

    int size = length / 2;
    Token **operators = malloc(size * sizeof(Token *));
    int operatorsEnd = 0;

    while (cur != NULL) {
        // Adds numbers, variables, functions to output stack
        if (cur->type != TOKEN_LEFT_PAREN && cur->type != TOKEN_RIGHT_PAREN) {
            if (cur->type == TOKEN_NUMBER || cur->type == TOKEN_IDENTIFIER || cur->type == TOKEN_FUNC_CALL) {
                output[outputEnd] = cur;
                outputEnd ++;
            } else {
                // At least one operator in, check precedence
                if (operatorsEnd > 0) {
                    if (getPrecedent(operators[operatorsEnd-1]->value) >= getPrecedent(cur->value)) {
                        // Flush operators to output stack
                        while (operatorsEnd > 0) {
                            output[outputEnd] = operators[operatorsEnd-1];
                            outputEnd ++;
                            operatorsEnd --;
                        }
                    } else {
                        if (operatorsEnd >= size) {
                            size *= 2;
                            Token **temp = realloc(operators, size * sizeof(Token *));
                            if (temp == NULL) {
                                printf("Cannot allocate more space for operator stack.");
                                free(operators);
                                free(output);

                                return (RPNList) {0, NULL};
                            }
                        }

                        operators[operatorsEnd] = cur;
                        operatorsEnd ++;
                    }
                } else {
                    operators[operatorsEnd] = cur;
                    operatorsEnd ++;
                }
            }
        }

        cur = cur->next;
    }

    // Flush remaining operators to output stack
    while (operatorsEnd > 0) {
        output[outputEnd] = operators[operatorsEnd-1];
        outputEnd++;
        operatorsEnd --;
    }
    
    free(operators);

    return (RPNList) {outputEnd, output};
}