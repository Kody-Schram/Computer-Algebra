#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/utils/log.h"


static const int DEFUALT_STRING_SIZE = 64;


Expression *dummyExpression(ExpressionType type) {
    Expression *expr = calloc(1, sizeof(Expression));

    if (expr == nullptr) {
        perror("Error in creating dummy expression");
        return nullptr;
    }

    expr->type = type;

    return expr;
}


Expression *deepCopyExpression(Expression *expr) {
    if (expr == nullptr) return nullptr;
    
    Expression *new = dummyExpression(expr->type);
    if (new == nullptr) return nullptr;

    switch (new->type) {
        case EXPRESSION_VARIABLE:
            new->identifier = strdup(expr->identifier);
            if (new->identifier == nullptr) goto error;
            break;

        case EXPRESSION_INTEGER:
            new->integer = expr->integer;
            break;

        case EXPRESSION_DOUBLE:
            new->value = expr->value;
            break;
        
        case EXPRESSION_OPERATOR:
            new->op = expr->op;
            new->arity = expr->arity;
            new->operands = calloc(expr->arity, sizeof(Expression *));
            if (new->operands == nullptr) goto error;
            
            for (int i = 0; i < expr->arity; i ++) {
                new->operands[i] = deepCopyExpression(expr->operands[i]);
                if (new->operands[i] == nullptr) {
                    // Frees already copied expressions
                    for (int j = 0; j < i; j ++) {
                        freeExpression(new->operands[j]);
                    }
                    free(new->operands);
                    goto error;
                }
            }
            
            break;

        case EXPRESSION_FUNCTION_CALL:
            FunctionCall *call = malloc(sizeof(FunctionCall));
            if (call == nullptr) goto error;
            
            call->identifier = strdup(expr->call->identifier);
            if (call->identifier == nullptr) goto error;
            
            call->nParams = expr->call->nParams;
            
            call->parameters = malloc(sizeof(Expression *) * call->nParams);
            if (call->parameters == nullptr) {
                free(call->identifier);
                free(call);
                
                goto error;
            }

            for (int i = 0; i < call->nParams; i ++) {
                call->parameters[i] = deepCopyExpression(expr->call->parameters[i]);
                if (call->parameters[i] == nullptr) {
                    free(call->identifier);
                    for (int j = 0; j < i; j ++) {
                        freeExpression(call->parameters[j]);
                    }
                    
                    free(call->parameters);
                    free(call);

                    goto error;
                }
            }

            new->call = call;
            break;

        default:
            break;
            
    }

    return new;

    error:
        perror("Error in copying expression");
        free(new);
        return nullptr;
}


static void printExpressionRec(Expression *expr, int level, FILE *stream) {
    if (expr == nullptr || stream == nullptr) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) fprintf(stream, "  ");

    switch(expr->type) {
        case EXPRESSION_INTEGER:
            fprintf(stream, "<type: INTEGER, value: %lld>\n", expr->integer);
            break;

        case EXPRESSION_DOUBLE:
            fprintf(stream, "<type: DOUBLE, value: %g>\n", expr->value);
            break;

        case EXPRESSION_OPERATOR:
            char id;
            fprintf(stream, "Operation: %c\n", expr->op->symbol);
            
            for (int i = 0; i < expr->arity; i ++) {
                printExpressionRec(expr->operands[i], level + 1, stream);
            }
            break;

        case EXPRESSION_VARIABLE:
            fprintf(stream, "<type: VARIABLE, identifier: '%s'>\n", expr->identifier);
            break;

        case EXPRESSION_FUNCTION_CALL: 
            if (expr->call == nullptr) {
                Debug(0, "Function call was nullptr\n");
                return;
            }
            if (expr->call->identifier == nullptr) {
                Debug(0, "Call identifer was nullptr\n");
                return;
            }
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", expr->call->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            fprintf(stream, "Parameters:\n");
            for (int p = 0; p < expr->call->nParams; p ++) printExpressionRec(expr->call->parameters[p], level + 1, stream);
            break;

        default:
            fprintf(stream, "no %d\n", expr->type);
    }
}


FILE *printExpression(Expression *expr) {
    FILE *stream = tmpfile();
    if (stream == nullptr) return nullptr;

    printExpressionRec(expr, 0, stream);
    return stream;
}


void freeExpression(Expression *expr) {
    if (expr == nullptr) return;

    switch (expr->type) {
        case EXPRESSION_FUNCTION_CALL:
            if (expr->call != nullptr) {
                free(expr->call->identifier);
                for (int i = 0; i < expr->call->nParams; i ++) {
                    freeExpression(expr->call->parameters[i]);
                }
                free(expr->call->parameters);
            }
            free(expr->call);
            break;

        case EXPRESSION_VARIABLE:
            free(expr->identifier);
            break;

        case EXPRESSION_OPERATOR:
            for (int i = 0; i < expr->arity; i ++) {
                freeExpression(expr->operands[i]);
            }
            free(expr->operands);
            break;
            
        default:
            break;
        
    }

    free(expr);
}


static void expressionToStringRecur(Expression *expr, FILE *stream) {
    if (expr == nullptr) return;

    switch (expr->type) {
        case EXPRESSION_OPERATOR:
            fprintf(stream, "(");
            for (int i = 0; i < expr->arity - 1; i ++) {
                expressionToStringRecur(expr->operands[i], stream);
                fprintf(stream, " %c ", expr->op->symbol);
            }

            expressionToStringRecur(expr->operands[expr->arity - 1], stream);
            fprintf(stream, ")");
            break;

        case EXPRESSION_DOUBLE:
            if (expr->value < 0) fprintf(stream, "(%g)", expr->value);
            else fprintf(stream, "%g", expr->value);
            break;

        case EXPRESSION_INTEGER:
            if (expr->integer < 0) fprintf(stream, "(%lld)", expr->integer);
            else fprintf(stream, "%lld", expr->integer);
            break;

        case EXPRESSION_VARIABLE:
            fprintf(stream, "%s", expr->identifier);
            break;

        case EXPRESSION_FUNCTION_CALL:
            fprintf(stream, "%s(", expr->call->identifier);
            for (int i = 0; i < expr->call->nParams - 1; i ++) {
                expressionToStringRecur(expr->call->parameters[i], stream);
                fprintf(stream, ", ");
            }
            expressionToStringRecur(expr->call->parameters[expr->call->nParams - 1], stream);
            fprintf(stream, ")");
            break;
            
        default:
            fprintf(stream, "How'd we get here?\n");
            break;
    }
}


char *expressionToString(Expression *expr) {
    FILE *stream = tmpfile();
    char *string = nullptr;
    if (expr == nullptr || stream == nullptr) {
        fclose(stream);
        return string;
    }
    expressionToStringRecur(expr, stream);
    
    long size = ftell(stream);
    if (size < 0) {
        fclose(stream);
        return nullptr;
    }

    string = malloc(size + 1);
    if (string == nullptr) {
        fclose(stream);
        return nullptr;
    }

    rewind(stream);

    size_t ptr = fread(string, 1, size, stream);
    string[ptr] = '\0';

    fclose(stream);
    return string;
}