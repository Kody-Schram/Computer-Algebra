#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parserUtils.h"
#include "utils/context/context.h"
#include "utils/log.h"


Token *createToken(TokenType type, char *value, int l) {
    //printf("creating token '%s' %d, of type %d\n", value, l, type);

    // Allocates new token
    Token *token = (Token*) calloc(1, sizeof(Token));
    if (token == NULL) {
        printf("Error allocating space for token.");
        return NULL;
    }

    // Populates newToken attributes
    token->type = type;
    token->value = strndup(value, l);
    token->next = NULL;

    return token;
}


static void printToken(Token *token, FILE *stream) {
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
            case TOKEN_MAPPING:
                type = "MAPPING";
                break;
            case TOKEN_SEPARATOR:
                type = "SEPERATOR";
                break;
            default:
                type = "FUNC_CALL";
        }

        if (token->type == TOKEN_FUNC_CALL) {
            fprintf(stream, "<type: %s>\n", type);
            //printf("<type: %s>\n", type);
        }
        else {
            fprintf(stream, "<type: %s, value: '%s'>\n", type, token->value);
            //printf(stream, "<type: %s, value: '%s'>\n", type, token->value);
        }
}


FILE *printTokens(Token *head) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    Token *cur = head;

    fprintf(stream, "\n");
    while (cur != NULL) {
        printToken(cur, stream);
        cur = cur->next;
    }

    fprintf(stream, "\n");
    return stream;
}


ASTNode *createASTNode(Token *token) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
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
        node->type = NODE_DOUBLE;
        char *end;
        double value = strtod(token->value, &end);
        
        if (value == (int) value) {
            node->type = NODE_INTEGER;
            node->integer = (int) value;
            break;
        }

        node->value = value;
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
                printf("Unknown operator '%c'\n", id);
        }
        break;
    case TOKEN_FUNC_CALL:
        node->type = NODE_FUNC_CALL;
        node->call = token->call;
        token->call = NULL;
        break;
    default:
        return NULL;
    }

    return node;
}


FILE *printRPN(RPNList *list) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    fprintf(stream, "RPN: ");

    for (int i = 0; i < list->length; i ++) {
        if (list->items[i]->type != TOKEN_FUNC_CALL) fprintf(stream, "%s ", list->items[i]->value);
        else fprintf(stream, "%s ", list->items[i]->call->identifier);
    }

    fprintf(stream, "\n");
    return stream;
}


void freeTokens(Token *head) {
    Debug(0, "Freeing tokens.\n");

    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        if (current->type != TOKEN_FUNC_CALL) free(current->value);
        free(current);
        current = next;
    }
}