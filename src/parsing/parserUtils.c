#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parserUtils.h"
#include "utils/context/context.h"
#include "utils/log.h"


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


void printToken(Token *token) {
    FILE *stream = GLOBALCONTEXT->config->LOG_STREAM;

    const char *type = NULL;

        switch(token->type) {
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

        if (token->type == TOKEN_FUNC_CALL) {
            fprintf(stream, "    <type: %s>\n", type);
        }
        else fprintf(stream, "    <type: %s, value: '%s'>\n", type, token->value);
}


void printTokens(Token *head) {
    FILE *stream = GLOBALCONTEXT->config->LOG_STREAM;

    Token *cur = head;

    fprintf(stream, "[\n");
    while (cur != NULL) {
        printToken(cur);
        cur = cur->next;
    }

    fprintf(stream, "]\n");
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
        char id = token->value[0];
        switch (id) {
            case '+':
                node->op = OP_ADDITION;
                break;
            case '-':
                node->op = OP_SUBTRACTION;
                break;
            case '*':
                node->op = OP_MULTIPLICATION;
                break;
            case '/':
                node->op = OP_DIVISION;
                break;
            case '^':
                node->op = OP_EXPONTENTIATION;
                break;
            default:
                printf("Unknown operator '%s'\n", id);
        }
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


void printRPN(RPNList *list) {
    FILE *stream = GLOBALCONTEXT->config->LOG_STREAM;

    fprintf(stream, "RPN: ");

    for (int i = 0; i < list->length; i ++) {
        if (list->items[i]->type != TOKEN_FUNC_CALL) fprintf(stream, "%s ", list->items[i]->value);
        else fprintf(stream, "%s ", list->items[i]->call->identifier);
    }

    fprintf(stream, "\n");
}


void freeTokens(Token *head) {
    Debug("Freeing tokens.\n");

    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }
}