#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser_utils.h"
#include "core/common.h"
#include "core/context.h"
#include "core/context/registry.h"
#include "core/parsing/parser_types.h"
#include "core/primitives/numbers.h"
#include "core/utils/log.h"
#include "core/utils/expr_utils.h"


Token *createOperatorToken(Operation const *op) {
    Token *token = calloc(1, sizeof(Token));
    if (token == NULL) {
        perror("Error creating token");
        return NULL;
    }
    
    token->type = TOKEN_OPERATOR;
	token->op = op;
    
    return token;
}


Token *createFuncCallToken(Component const *cmp) {
	Token *token = calloc(1, sizeof(Token));
	if (token == NULL) {
		perror("Error creating token");
		return NULL;
	}

	token->type = TOKEN_FUNC_CALL_PLACEHOLDER;
	token->cmp = cmp;

	return token;
}


Token *createToken(TokenType type, char const *value, int l) {
    if (type != TOKEN_OPERATOR) {
        // Allocates new token
        Token *token = (Token*) calloc(1, sizeof(Token));
        if (token == NULL) {
            perror("Error creating token");
            return NULL;
        }
    
        // Populates newToken attributes
        token->type = type;
        
		if (token->type != TOKEN_LEFT_PAREN  && token->type != TOKEN_RIGHT_PAREN
				&& token->type != TOKEN_ASSIGNMENT && token->type != TOKEN_MAPPING && token->type != TOKEN_SEPARATOR) {

			token->value = malloc(l + 1);
			if (token->value == NULL) {
				perror("Error creating token");
				free(token);
				return NULL;
			} 
		
			memcpy(token->value, value, l);
			token->value[l] = '\0';

		}

		return token;
        
    } else {
        Debug(0, "Creating operator token, %c\n", value[0]);
		const Operation *op = searchOperation(GLOBALCONTEXT->registry, value[0]);
        if (op == NULL) {
            return NULL;
        }

        return createOperatorToken(op);
        
    }
}


static void printToken(Token const *token, FILE *stream) {
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

        if (token->type == TOKEN_FUNC_CALL || token->type == TOKEN_FUNC_CALL_PLACEHOLDER ) {
            fprintf(stream, "<type: %s>\n", type);
            //printf("<type: %s>\n", type);
        }
        else if (token->type == TOKEN_OPERATOR) {
            fprintf(stream, "<type: %s, symbol: '%c'>\n", type, token->op->symbol);
        } else if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER) {
            fprintf(stream, "<type: %s, value: '%s'>\n", type, token->value);
		} else {
            fprintf(stream, "<type: %s>\n", type);
        }
}


FILE *printTokens(Token const *head) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    Token const *cur = head;

    fprintf(stream, "\n");
    while (cur != NULL) {
        printToken(cur, stream);
        cur = cur->next;
    }

    fprintf(stream, "\n");
    return stream;
}


Expression *createExpression(Token *token) {
	Expression *expr;

	if (token->type != TOKEN_FUNC_CALL) {
		expr = calloc(1, sizeof(Expression));
		if (expr == NULL) goto error;
	}

    switch (token->type) {
        case TOKEN_IDENTIFIER:
            expr->type = EXPRESSION_VARIABLE;
            expr->identifier = token->value;
            break;
            
        case TOKEN_NUMBER:
            expr->type = EXPRESSION_OBJECT;
			expr->objectId = NUMBER_ID;
			if (!GLOBALCONTEXT->registry->numberParser(token->value, &expr->value, &expr->flags)) {
				free(token->value);
				goto error;
			}

			free(token->value);
            break;
            
        case TOKEN_OPERATOR:
            expr->type = EXPRESSION_OPERATOR;
            expr->op = token->op;
            expr->nOperands = 2;
            
            expr->operands = malloc(sizeof(Expression *) * expr->nOperands);
            if (expr->operands == NULL) goto error;

            break;
            
        case TOKEN_FUNC_CALL:
			expr = token->finalizedCall;
			Debug(0, "new expr fn nparams %d\n", expr->cmp->func->nParameters);
            break;
            
        default:
            return NULL;
    }

	token->value = NULL;

    return expr;
    
    error:
        perror("Error creating expression");
        freeExpression(expr);
        return NULL;
}


FILE *printRPN(RPNList const *list) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    fprintf(stream, "RPN: ");

    for (uint32_t i = 0; i < list->length; i ++) {
        if (list->items[i]->type != TOKEN_FUNC_CALL && list->items[i]->type != TOKEN_OPERATOR) fprintf(stream, "%s ", list->items[i]->value);
        else if (list->items[i]->type == TOKEN_OPERATOR) fprintf(stream, "%c ", list->items[i]->op->symbol);
        else fprintf(stream, "%s(%d)", list->items[i]->finalizedCall->cmp->identifier, list->items[i]->finalizedCall->cmp->func->nParameters);
    }

    fprintf(stream, "\n");
    return stream;
}


void freeTokens(Token *head) {
    Debug(0, "Freeing tokens.\n");

    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
        free(current);
        current = next;
    }
}


void deepFreeTokens(Token *head) {
    Debug(0, "Deep Freeing tokens.\n");

    Token* current = head;
    while (current != NULL) {
        Token *next = current->next;
		if (current->type != TOKEN_OPERATOR  && 
			current->type != TOKEN_FUNC_CALL_PLACEHOLDER  &&
			current->type != TOKEN_FUNC_CALL) 
		{
			free(current->value);
		}

        free(current);
        current = next;
    }
}
