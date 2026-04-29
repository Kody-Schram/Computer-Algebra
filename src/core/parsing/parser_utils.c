#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser_utils.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/parsing/parser_types.h"
#include "core/utils/log.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"


Token *createTokenOperator(const Operation *op) {
    Debug(0, "running dedicated operator creation func\n");
    Token *token = calloc(1, sizeof(Token));
    if (token == NULL) {
        perror("Error creating token");
        return NULL;
    }
    
    token->type = TOKEN_OPERATOR;
    token->next = NULL;
	token->op = op;
    
	Debug(0, "returning new operator token\n");
    return token;
}


Token *createTokenFuncPlaceholder(const Function *func) {
	Debug(0, "Running dedicated function placeholder creation func\n");
	Token *token = calloc(1, sizeof(Token));
	if (token == NULL) {
		perror("Error creating token");
		return NULL;
	}

	token->type = TOKEN_FUNC_CALL_PLACEHOLDER;
	token->next = NULL;
	token->func = func;

	return token;
}


Token *createToken(TokenType type, const char *value, int l) {
    if (type != TOKEN_OPERATOR) {
        // Allocates new token
        Token *token = (Token*) calloc(1, sizeof(Token));
        if (token == NULL) {
            perror("Error creating token");
            return NULL;
        }
    
        // Populates newToken attributes
        token->type = type;
        
        token->value = malloc(l + 1);
        if (token->value == NULL) {
            perror("Error creating token");
            free(token);
            return NULL;
        } 
    
        memcpy(token->value, value, l);
        token->value[l] = '\0';
        token->next = NULL;
        
        return token;
    } else {
        Debug(0, "Creating operator token, %c\n", value[0]);
        Component *cmp = searchEnvironmentOperator(GLOBALCONTEXT->env, value[0]);
        if (cmp == NULL) {
            return NULL;
        }

        return createTokenOperator(cmp->operation);
        
    }
}


static void printToken(const Token *token, FILE *stream) {
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
                type = "SEPARATOR";
                break;
			case TOKEN_FUNC_CALL_PLACEHOLDER:
				type = "FUNC_CALL_PLACEHOLDER";
				break;
            case TOKEN_FUNC_CALL:
                type = "FUNC_CALL";
				break;
			default:
				type = "Define this token type";
				break;
        }

        if (token->type == TOKEN_FUNC_CALL) {
            fprintf(stream, "<type: %s>\n", type);
            //printf("<type: %s>\n", type);
        }
        else if (token->type == TOKEN_OPERATOR) {
            fprintf(stream, "<type: %s, symbol: '%c'>\n", type, token->op->symbol);
        }
        else {
            fprintf(stream, "<type: %s, value: '%s'>\n", type, token->value);
            //printf(stream, "<type: %s, value: '%s'>\n", type, token->value);
        }
}


FILE *printTokens(const Token *head) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    const Token *cur = head;

    fprintf(stream, "\n");
    while (cur != NULL) {
        printToken(cur, stream);
        cur = cur->next;
    }

    fprintf(stream, "\n");
    return stream;
}


Expression *createExpression(const Token *token) {
    Expression *expr = calloc(1, sizeof(Expression));
    if (expr == NULL) goto error;

    switch (token->type) {
        case TOKEN_IDENTIFIER:
            expr->type = EXPRESSION_VARIABLE;
            expr->identifier = strdup(token->value);
            if (expr->identifier == NULL) goto error;
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
            expr->op = token->op;
            expr->nOperands = expr->op->arity;
            
            expr->operands = malloc(sizeof(Expression *) * expr->nOperands);
            if (expr->operands == NULL) goto error;
            break;
            
        case TOKEN_FUNC_CALL:
            expr->type = EXPRESSION_FUNCTION_CALL;
            expr->call = token->call;
            break;
            
        default:
            return NULL;
    }

    return expr;
    
    error:
        perror("Error creating expression");
        freeExpression(expr);
        return NULL;
}


FILE *printRPN(const RPNList *list) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    fprintf(stream, "RPN: ");

    for (int i = 0; i < list->length; i ++) {
        if (list->items[i]->type != TOKEN_FUNC_CALL && list->items[i]->type != TOKEN_OPERATOR) fprintf(stream, "%s ", list->items[i]->value);
        else if (list->items[i]->type == TOKEN_OPERATOR) fprintf(stream, "%c ", list->items[i]->op->symbol);
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
        
        if (current->type != TOKEN_FUNC_CALL && current->type != TOKEN_OPERATOR) {
            Debug(0, "Freeing '%s'\n", current->value);
            free(current->value);
        }
        else Debug(0, "Freeing function call or operator\n");

        free(current);
        current = next;
    }
}
