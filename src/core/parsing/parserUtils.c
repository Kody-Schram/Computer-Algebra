#include <stdlib.h>
#include <string.h>

#include "parserUtils.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


Token *createToken(TokenType type, char *value, int l) {
    // Allocates new token
    Token *token = (Token*) calloc(1, sizeof(Token));
    if (token == nullptr) {
        printf("Error allocating space for token.");
        return nullptr;
    }

    // Populates newToken attributes
    token->type = type;

    token->value = malloc(l + 1);
    if (token->value == nullptr) {
        printf("Error allocating for token value.\n");
        free(token);
        return nullptr;
    } 

    memcpy(token->value, value, l);
    token->value[l] = '\0';

    token->next = nullptr;

    return token;
}


static void printToken(Token *token, FILE *stream) {
    const char *type = nullptr;

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
                type = "SEPARATOR";
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
    if (stream == nullptr) return nullptr;

    Token *cur = head;

    fprintf(stream, "\n");
    while (cur != nullptr) {
        printToken(cur, stream);
        cur = cur->next;
    }

    fprintf(stream, "\n");
    return stream;
}


Expression *createExpression(Token *token) {
    Expression *expr = calloc(1, sizeof(Expression));
    if (expr == nullptr) {
        printf("Error allocating for new node.\n");
        return nullptr;
    }

    switch (token->type)
    {
    case TOKEN_IDENTIFIER:
        expr->type = EXPRESSION_VARIABLE;
        expr->identifier = strdup(token->value);
        if (expr->identifier == nullptr) {
            printf("Error copying variable identifier.\n");
            return nullptr;
        }
        break;
    case TOKEN_NUMBER:
        expr->type = EXPRESSION_DOUBLE;
        char *end;
        double value = strtod(token->value, &end);
        
        if (value == (int) value) {
            expr->type = EXPRESSION_INTEGER;
            expr->integer = (long long) value;
            break;
        }

        node->value = value;
        break;
    case TOKEN_OPERATOR:
        expr->type = EXPRESSION_OPERATOR;
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
        token->call = nullptr;
        break;
    default:
        return nullptr;
    }

    return expr;
}


FILE *printRPN(RPNList *list) {
    FILE *stream = tmpfile();
    if (stream == nullptr) return nullptr;

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
    while (current != nullptr) {
        Token *next = current->next;
        
        if (current->type != TOKEN_FUNC_CALL) {
            Debug(0, "Freeing '%s'\n", current->value);
            free(current->value);
        }
        else Debug(0, "Freeing function call\n");

        free(current);
        current = next;
    }
}