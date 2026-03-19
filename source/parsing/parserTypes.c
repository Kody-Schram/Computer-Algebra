#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "parserTypes.h"


Token *createToken(TokenType type, char *value, int l) {
    //printf("creating token '%s' %d, of type %d\n", value, l, type);

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
            case TOKEN_FUNC_DEF:
                type = "FUNC_DEF";
                break;
            case TOKEN_SEPERATOR:
                type = "SEPERATOR";
                break;
            default:
                type = "FUNC_CALL";
        }

        if (cur->type == TOKEN_FUNC_CALL) {
            //printf("printing funciton \n");
            printf("    <type: %s>\n", type);
            // if (cur->call != NULL) {
            //     printf("    <type: %s, value: '%s'>\n", type, cur->call->identifier);
            // } else {
            //     printf("call not defined\n");
            //     printf("    <type: %s, value: '%s'>\n", type, cur->value);
            // }
        }
        else printf("    <type: %s, value: '%s'>\n", type, cur->value);
        cur = cur->next;
    }

    printf("]\n");
}


ASTNode *createASTNode(Token *token) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        printf("Error allocating for new node.\n");
        return NULL;
    }

    switch (token->type)
    {
    case TOKEN_IDENTIFIER:
        node->type = NODE_VARIABLE;
        node->identifier = strdup(token->value);
        break;
    case TOKEN_NUMBER:
        node->type = NODE_NUMBER;
        char *end;
        node->value = strtod(token->value, &end);
        break;
    case TOKEN_OPERATOR:
        node->type = NODE_OPERATOR;
        node->identifier = strdup(token->value);
        break;
    case TOKEN_FUNC_CALL:
        node->type = NODE_FUNC_CALL;
        node->identifier = strdup(token->value);
        node->call = token->call;
        break;
    default:
        return NULL;
    }

    node->left = NULL;
    node->right = NULL;
    return node;
}


ASTNode *dummyASTNode(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        printf("Error allocating for new node.\n");
        return NULL;
    }

    node->type = type;
    node->left = NULL;
    node->right = NULL;

    return node;
}


static void printASTRec(ASTNode *node, int level) {
    if (node == NULL) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) printf("  ");

    // Print node info
    switch(node->type) {
        case NODE_NUMBER:
            printf("<type: %s, value: %f>\n", "NUMBER", node->value);
            break;
        case NODE_OPERATOR:
            printf("<type: %s, symbol: %s>\n", "OPERATOR", node->identifier);
            break;
        case NODE_VARIABLE:
            printf("<type: %s, identifier: %s>\n", "VARIABLE", node->identifier);
            break;
        case NODE_FUNC_CALL: 
            printf("<type: %s, value: '%s'>\n", "FUNC_CALL", node->call->identifier);
            break;
        default:
            printf("no\n");
    }

    // Recursively print children
    printASTRec(node->left, level + 1);
    printASTRec(node->right, level + 1);
}


void printAST(ASTNode *root) {
    printASTRec(root, 0);
}