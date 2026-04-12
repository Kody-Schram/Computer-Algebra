#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parserUtils.h"
#include "core/utils/context/context.h"
#include "core/utils/context/environment.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


Token *createToken(TokenType type, char *value, int l) {
    // Allocates new token
    Token *token = (Token*) calloc(1, sizeof(Token));
    if (token == nullptr) {
        perror("Error creating token");
        return nullptr;
    }

    // Populates newToken attributes
    token->type = type;

    token->value = malloc(l + 1);
    if (token->value == nullptr) {
        perror("Error creating token");
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
        perror("Error creating expression");
        return nullptr;
    }

    switch (token->type) {
        case TOKEN_IDENTIFIER:
            expr->type = EXPRESSION_VARIABLE;
            expr->identifier = strdup(token->value);
            if (expr->identifier == nullptr) {
                perror("Error creating expression");
                return nullptr;
            }
            break;
            
        case TOKEN_NUMBER:
            expr->type = EXPRESSION_DOUBLE;
            char *end;
            double value = strtod(token->value, &end);
            
            if (value == (long long) value) {
                expr->type = EXPRESSION_INTEGER;
                expr->integer = (long long) value;
                break;
            }
    
            expr->value = value;
            break;
            
        case TOKEN_OPERATOR:
            expr->type = EXPRESSION_OPERATOR;
            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, token->value);
            if (cmp == nullptr || cmp->type != COMP_OPERATION) {
                printf("Couldn't locate operation %s in environment.\n", token->value);
                return nullptr;
            }
            expr->op = cmp->operation;
            expr->arity = 2;
            
            // By default starts as binary operation, can be flattened and resized later
            expr->operands = malloc(sizeof(Expression *) * 2);
            if (expr->operands == nullptr) {
                perror("Error creating expression");
                return nullptr;
            }
            break;
            
        case TOKEN_FUNC_CALL:
            expr->type = EXPRESSION_FUNCTION_CALL;
            expr->call = token->call;
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