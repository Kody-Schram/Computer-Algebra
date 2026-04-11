#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "core/utils/context/context.h"
#include "core/utils/log.h"
#include "core/parsing/parserUtils.h"
#include "core/utils/types.h"

static int DEFAULT_NODE_STACK_SIZE = 10;

/**
 * @brief Gets the precedent of operator
 * 
 * @param operator 
 * @return int Precedent of operator
 */
static int getPrecedent(char *operator) {
    int precedent = 0;

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
    while (cur != nullptr) {
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
    if (temp == nullptr) {
        printf("Error reallocating space for stack.\n");
        return 1;
    }

    stack->items = temp;
    return 0;
}


RPNList *shuntingYard(Token *head) {
    Config *config = GLOBALCONTEXT->config;

    Debug(0, "\nCreating RPN List.\n");
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
        malloc(size / 2 * sizeof(Token *))
    };

    RPNList *list = nullptr;

    if (output.items == nullptr || operators.items == nullptr) goto error;

    while (cur != nullptr) {
        // Add numbers, identifiers, and function calls to output stack
        if (cur->type == TOKEN_NUMBER || cur->type == TOKEN_IDENTIFIER || cur->type == TOKEN_FUNC_CALL) {
            output.items[output.entries] = cur;
            output.entries ++;

        // Adds opening parenthesis to operators
        } else if (cur->type == TOKEN_LEFT_PAREN) {
            operators.items[operators.entries] = cur;
            operators.entries ++;

            if (operators.entries == operators.size) {
                if (reallocStack(&operators)) goto error;
            }

        } else if (cur->type == TOKEN_OPERATOR) {
            if (operators.entries > 0) {
                // Pops all operators on stack with greater or equal precedent
                while ((operators.entries > 0) 
                    && (((Token *) operators.items[operators.entries - 1])->value[0] != '(') 
                    && (getPrecedent(((Token *) operators.items[operators.entries - 1])->value) >= getPrecedent(cur->value))) {
                    
                            // Right associativity for exponents
                    if (((Token *) operators.items[operators.entries - 1])->value[0] == '^' 
                        && getPrecedent(((Token *) operators.items[operators.entries - 1])->value) == getPrecedent(cur->value)) break;
                    
                    operators.entries --;
                    output.items[output.entries] = operators.items[operators.entries];
                    output.entries ++;
                }
            }

            operators.items[operators.entries] = cur;
            operators.entries ++;

            if (operators.entries == operators.size) {
                if (reallocStack(&operators)) goto error;
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

    list = malloc(sizeof(RPNList));
    if (list == nullptr) goto error;

    list->length = output.entries;
    list->items = (Token**) output.items;

    Debug(0, "RPN List\n");
    Debug(1, printRPN(list));

    return list;

    error:
        free(output.items);
        free(operators.items);
        free(list);

        return nullptr;
}


Expression *expressionFromRPN(RPNList *rpn) {
    // Look into block allocation to pack nodes nearby, a very good estimate on the size of the block needed comes from the size of the token list
    Debug(0, "\nGenerating AST.\n");

    Stack expressions = {
        DEFAULT_NODE_STACK_SIZE,
        0,
        malloc(DEFAULT_NODE_STACK_SIZE * sizeof(Expression *))
    };
    if (expressions.items == nullptr) return nullptr;
    Expression *root = nullptr;

    for (int i = 0; i < rpn->length; i ++) {
        Expression *expr = createExpression(rpn->items[i]);
        if (expr == nullptr) goto cleanup;

        if (expr->type == EXPRESSION_OPERATOR) {

            if (expressions.entries < 2) {
                printf("Unexpected error in AST generation.\n");
                goto cleanup;
            }
            expr->operands[0] = expressions.items[expressions.entries - 1];
            expr->operands[1] = expressions.items[expressions.entries - 2];

            expressions.entries -= 2;
        }

        expressions.items[expressions.entries] = expr;
        expressions.entries ++;
        if (expressions.entries >= expressions.size) {
            if (reallocStack(&expressions)) goto cleanup;
        }
    }

    root = expressions.items[0];
    free(expressions.items);

    return root;

    cleanup:
        freeExpression(root);
        free(expressions.items);
        return nullptr;
}