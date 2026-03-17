#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "parserTypes.h"

Token *createToken(TokenType type, char *value, int l) {
    // Allocates new token
    Token *token = (Token*) malloc(sizeof(Token));
    if (token == NULL) {
        printf("Error allocating space for token.");
        return NULL;
    }

    // Populates newToken attributes
    token->type = type;
    token->value = (char*) malloc(l+1);
    if (token->value == NULL) {
        printf("Error allocating space for token value.");
        return NULL;
    }
    memcpy(token->value, value, l);
    token->value[l] = '\0';
    token->next = NULL;

    return token;
}

void printTokens(Token *head) {
    Token *cur = head;

    printf("[\n");
    while (cur != NULL) {
        const char *type = NULL;

        switch(cur->type) {
            case TOKEN_NUMBER:
                type = "NUMBER";
                break;
            case TOKEN_OPERATOR:
                type = "OPERATOR";
                break;
            case TOKEN_IDENTIFIER:
                type = "IDENTIFIER";
                break;
            case TOKEN_ASSIGNMENT:
                type = "ASSIGNMENT";
                break;
            case TOKEN_LEFT_PAREN:
                type = "LEFT_PAREN";
                break;
            case TOKEN_RIGHT_PAREN:
                type = "RIGHT_PAREN";
                break;
            case TOKEN_FUNC_CALL:
                type = "FUNC_CALL";
                break;
            case TOKEN_FUNC_DEF:
                type = "FUNC_DEF";
                break;
            case TOKEN_SEPERATOR:
                type = "SEPERATOR";
                break;
        }

        printf("    <type: %s, value: '%s'>\n", type, cur->value);
        cur = cur->next;
    }

    printf("]\n");
}

ASTNode *createASTNode(Token *token, Environment *env) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        printf("Error allocating for new node.\n");
        return NULL;
    }

    switch (token->type)
    {
    case TOKEN_IDENTIFIER:
        node->type = NODE_VARIABLE;
        node->identifier = token->value;
        break;
    case TOKEN_NUMBER:
        node->type = NODE_NUMBER;
        char *end;
        node->value = strtod(token->value, &end);
        break;
    case TOKEN_OPERATOR:
        node->type = NODE_OPERATOR;
        node->identifier = token->value;
        break;
    case TOKEN_FUNC_CALL:
        node->type = NODE_FUNC_CALL;

        // Maps identifier to function definition
        Component *component = searchEnvironment(env, token->value);
        Function *func = component->function;
        if (func == NULL) return NULL;

        node->function = func;
        break;
    default:
        return NULL;
    }

    node->left = NULL;
    node->right = NULL;
    return node;
}

void printASTRec(ASTNode *node) {
    if (node == NULL) return;

    // print current node
    switch (node->type) {
        case NODE_NUMBER:
            printf("Num: %f", node->value);
            break;
        case NODE_OPERATOR:
            printf("OP: %s", node->identifier);
            break;
        case NODE_VARIABLE:
            printf("VAR: %s", node->identifier);
            break;
        // case NODE_FUNC_CALL:
        //     printf("FNC: %s", node->function->identifier);
        //     break;
        // case NODE_FUNC_DEF:
        //     printf("FND: %s", node->function->identifier);
        //     break;
        default:
            printf("?");
            break;
    }

    printf(", ");

    printASTRec(node->left);
    printASTRec(node->right);
}


void printAST(ASTNode *root) {
    printASTRec(root);
}