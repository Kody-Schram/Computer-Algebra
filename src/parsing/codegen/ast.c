#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "parsing/parserUtils.h"

static int DEFAULT_NODE_STACK_SIZE = 10;

/**
 * @brief Gets the precedent of operator
 * 
 * @param operator 
 * @return int Precedent of operator
 */
static int getPrecedent(char *operator) {
    int precedent;

    // set precedent for '->'
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


/**
 * @brief Gets the length of linked list
 * 
 * @param head Start of Token linked list
 * @return int Length of linked list
 */
static int getLinkedListLength(Token *head) {
    Token *cur = head;

    int i = 0;
    while (cur != NULL) {
        i ++;
        cur = cur->next;
    }

    return i;
}

/**
 * @brief Reallocates a stack
 * 
 * @retval 0: Successfully reallocated
 * @retval 1: Error reallocating
 * 
 * @param stack 
 * @return int Result
 */
static int reallocStack(Stack *stack) {
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
                if (reallocStack(&operators)) return NULL;
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
                if (reallocStack(&operators)) return NULL;
            }

        } else if (cur->type == TOKEN_RIGHT_PAREN) {
            // Pops all operators within parenthesis when closing is reached
            while (operators.entries > 0 && ((Token *) operators.items[operators.entries - 1])->value[0] != '(') {
                operators.entries --;
                output.items[output.entries] = operators.items[operators.entries];
                output.entries ++;
            }

            // Removes '(' from list
            operators.entries --;
        }

        cur = cur->next;
    }

    // Flush remaining operators to output stack
    while (operators.entries > 0) {
        //printf("flushing operators\n");
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
    list->items = (Token**) output.items;

    return list;
}


ASTNode *astFromRPN(RPNList *rpn) {
    Stack nodes = {
        DEFAULT_NODE_STACK_SIZE,
        0,
        malloc(DEFAULT_NODE_STACK_SIZE * sizeof(ASTNode *))
    };

    for (int i = 0; i < rpn->length; i ++) {
        //printf("creating new node. %s\n", rpn->items[i]->value);
        ASTNode *node = createASTNode(rpn->items[i]);
        if (node == NULL) return NULL;

        if (node->type == NODE_OPERATOR) {
            //printf("adding children\n");

            node->right = nodes.items[nodes.entries - 1];
            node->left = nodes.items[nodes.entries - 2];

            // printf("left %s", node->left->identifier);
            // printf("right %s", node->right->value);

            nodes.entries -= 2;
        }

        nodes.items[nodes.entries] = node;
        nodes.entries ++;
        if (nodes.entries >= nodes.size) {
            if (reallocStack(&nodes)) return NULL;
        }
    }

    //printf("length %d\n", nodes.entries);
    return nodes.items[0];
}