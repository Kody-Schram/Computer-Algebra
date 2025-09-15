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
        case '^':
            precedent = 3;
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


int reallocStack(Stack *stack) {
    stack->size *= 2;
    void **temp = realloc(stack->items, stack->size * sizeof(void*));
    if (temp == NULL) {
        printf("Error reallocating space for stack.\n");
        return 1;
    }

    stack->items = temp;
    return 0;
}

RPNList *shuntingYard(Token *head) {
    Token *cur = head;

    int size = getLinkedListLength(head);
    Stack output = {
        size,
        0,
        malloc(size * sizeof(Token *))
    };

    Stack operators = {
        size / 2,
        0,
        malloc(size /2 * sizeof(Token *))
    };

    while (cur != NULL) {
        // Add numbers, identifiers, and function calls to output stack
        if (cur->type == TOKEN_NUMBER || cur->type == TOKEN_IDENTIFIER || cur->type == TOKEN_FUNC_CALL) {
            output.items[output.entries] = cur;
            output.entries ++;

        // Adds opening parenthesis to operators
        } else if (cur->type == TOKEN_LEFT_PAREN) {
            operators.items[operators.entries] = cur;
            operators.entries ++;

            if (operators.entries == operators.size) {
                reallocStack(&operators);
            }

        } else if (cur->type == TOKEN_OPERATOR) {
            if (operators.entries > 0) {
                // Pops all operators on stack with greater or equal precedent
                while ((operators.entries > 0) 
                        && (((Token *) operators.items[operators.entries - 1])->value[0] != '(') 
                        && (getPrecedent(((Token *) operators.items[operators.entries - 1])->value) >= getPrecedent(cur->value))) {

                    operators.entries --;
                    output.items[output.entries] = operators.items[operators.entries];
                    output.entries ++;
                }
            }

            operators.items[operators.entries] = cur;
            operators.entries ++;

            if (operators.entries == operators.size) {
                reallocStack(&operators);
            }

        } else if (cur->type == TOKEN_RIGHT_PAREN) {
            // Pops all operators within parenthesis when closing is reached
            while (operators.entries > 0 && ((Token *) operators.items[operators.entries - 1])->value[0] != '(') {
                operators.entries --;
                output.items[output.entries] = operators.items[operators.entries];
                output.entries ++;
            }
        }

        cur = cur->next;
    }

    // Flush remaining operators to output stack
    while (operators.entries > 0) {
        printf("flushing operators\n");
        output.items[output.entries] = operators.items[operators.entries-1];
        output.entries++;
        operators.entries --;
    }
    
    free(operators.items);

    RPNList *list = malloc(sizeof(RPNList));
    if (list == NULL) {
        printf("Error allocating for RPN List.\n");
    }

    list->length = output.entries;
    list->rpn = (Token**) output.items;

    return list;
}

// ASTNode *astFromRPN(RPNList rpn) {
//     return NULL;
// }

// ASTNode *generateAST(Token *head) {
//     RPNList *rpn = shuntingYard(head);
//     if (rpn->length == 0) return NULL;

//     ASTNode *ast = astFromRPN(*rpn);
//     return ast;
// }