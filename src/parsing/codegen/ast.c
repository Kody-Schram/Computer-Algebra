#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "utils/context/context.h"
#include "utils/log.h"
#include "parsing/parserUtils.h"

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


ASTNode *astFromRPN(RPNList *rpn) {
    // Look into block allocation to pack nodes nearby, a very good estimate on the size of the block needed comes from the size of the token list
    Debug(0, "\nGenerating AST.\n");

    Stack nodes = {
        DEFAULT_NODE_STACK_SIZE,
        0,
        malloc(DEFAULT_NODE_STACK_SIZE * sizeof(ASTNode *))
    };
    if (nodes.items == nullptr) return nullptr;
    ASTNode *ast = nullptr;

    for (int i = 0; i < rpn->length; i ++) {
        ASTNode *node = createASTNode(rpn->items[i]);
        if (node == nullptr) goto cleanup;

        if (node->type == NODE_OPERATOR) {

            if (nodes.entries < 2) {
                printf("Unexpected error in AST generation.\n");
                goto cleanup;
            }
            node->right = nodes.items[nodes.entries - 1];
            node->left = nodes.items[nodes.entries - 2];

            nodes.entries -= 2;
        }

        nodes.items[nodes.entries] = node;
        nodes.entries ++;
        if (nodes.entries >= nodes.size) {
            if (reallocStack(&nodes)) goto cleanup;
        }
    }

    ast = nodes.items[0];
    free(nodes.items);

    return ast;

    cleanup:
        freeAST(ast);
        free(nodes.items);
        return nullptr;
}