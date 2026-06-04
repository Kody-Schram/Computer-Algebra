#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "expr_utils.h"
#include "core/context.h"
#include "core/utils/log.h"


#define DEFUALT_STRING_SIZE 64


Expression *dummyExpression(ExpressionType type) {
    Expression *expr = calloc(1, sizeof(Expression));

    if (expr == NULL) {
        perror("Error in creating dummy expression");
        return NULL;
    }

    expr->type = type;

    return expr;
}


Expression *deepCopyExpression(Expression const *expr) {
    if (expr == NULL) return NULL;
    
    Expression *new = dummyExpression(expr->type);
    if (new == NULL) return NULL;

    switch (new->type) {
        case EXPRESSION_VARIABLE:
            new->identifier = strdup(expr->identifier);
            if (new->identifier == NULL) goto error;

            break;

		case EXPRESSION_OBJECT:
			new->objectId = expr->objectId;
			Object const *obj = searchObject(GLOBALCONTEXT->registry, new->objectId);
			if (obj == NULL) goto error;

			new->data = obj->copy(expr->data);

			break;

        case EXPRESSION_INTEGER:
            new->integer = expr->integer;
            break;

        case EXPRESSION_DOUBLE:
            new->value = expr->value;
            break;
        
        case EXPRESSION_OPERATOR:
            new->op = expr->op;
            new->nOperands = expr->nOperands;
            new->operands = calloc(expr->nOperands, sizeof(Expression *));
            if (new->operands == NULL) goto error;
            
            for (int i = 0; i < expr->nOperands; i ++) {
                new->operands[i] = deepCopyExpression(expr->operands[i]);
                if (new->operands[i] == NULL) {
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
           	new->cmp = expr->cmp;
            new->nInputs = expr->nInputs;

            new->inputs = malloc(sizeof(Expression *) * expr->nInputs);
            if (new->inputs == NULL) goto error;

            for (uint32_t i = 0; i < expr->nInputs; i ++) {
                new->inputs[i] = deepCopyExpression(expr->inputs[i]);
                if (new->inputs[i] == NULL) {
                    for (uint32_t j = 0; j < i; j ++) {
                        freeExpression(new->inputs[j]);
                    }
                    
                    free(new->inputs);

                    goto error;
                }
            }

            break;

        default:
            break;
            
    }

    return new;

    error:
        perror("Error in copying expression");
        free(new);
        return NULL;
}


static void printExpressionRec(Expression const *expr, int level, FILE *stream) {
    if (expr == NULL || stream == NULL) return;

    // Print indentation based on depth
    for (int i = 0; i < level; i++) fprintf(stream, "  ");

    switch(expr->type) {
        case EXPRESSION_INTEGER:
            fprintf(stream, "<type: INTEGER, value: %" PRId64 ">\n", expr->integer);
            break;

        case EXPRESSION_DOUBLE:
            fprintf(stream, "<type: DOUBLE, value: %g>\n", expr->value);
            break;

        case EXPRESSION_OPERATOR:
            char id;
            fprintf(stream, "Operation: %c\n", expr->op->symbol);
            
            for (int i = 0; i < expr->nOperands; i ++) {
                printExpressionRec(expr->operands[i], level + 1, stream);
            }
            break;

        case EXPRESSION_VARIABLE:
            fprintf(stream, "<type: VARIABLE, identifier: '%s'>\n", expr->identifier);
            break;

        case EXPRESSION_FUNCTION_CALL: 
            fprintf(stream, "<type: FUNC_CALL, value: '%s'>\n", expr->cmp->identifier);
            for (int i = 0; i < level + 1; i++) fprintf(stream, "  ");

            fprintf(stream, "Parameters:\n");
            for (int p = 0; p < expr->nInputs; p ++) printExpressionRec(expr->inputs[p], level + 1, stream);
            break;

		case EXPRESSION_OBJECT:
			Object const *obj = searchObject(GLOBALCONTEXT->registry, expr->objectId);
			if (obj != NULL && obj->print != NULL) {
				char *out = obj->print(expr->data);
				fprintf(stream, "<type: OBJECT, data: %s>\n", out);
				free(out);
			}

			else fprintf(stream, "<type: OBJECT, data: ?>\n");

			break;

        default:
            fprintf(stream, "no %d\n", expr->type);
    }
}


FILE *printExpression(const Expression *expr) {
    FILE *stream = tmpfile();
    if (stream == NULL) return NULL;

    printExpressionRec(expr, 0, stream);
    return stream;
}


void freeExpression(Expression *expr) {
    if (expr == NULL) return;
    Debug(0, "Freeing expression\n");

    switch (expr->type) {
        case EXPRESSION_FUNCTION_CALL:
			// for (uint32_t i = 0; i < expr->nInputs; i ++) {
			// 	freeExpression(expr->inputs[i]);
			// }
			free(expr->inputs);
            break;

        case EXPRESSION_VARIABLE:
            free(expr->identifier);
            break;

        case EXPRESSION_OPERATOR:
            for (int i = 0; i < expr->nOperands; i ++) {
                freeExpression(expr->operands[i]);
            }
            free(expr->operands);
            break;

		case EXPRESSION_OBJECT:
			Object const *obj = searchObject(GLOBALCONTEXT->registry, expr->objectId);
			if (obj != NULL && obj->cleanup != NULL) obj->cleanup(expr->data);	

			break;
            
        default:
            break;
        
    }

    free(expr);
}


static void expressionToStringRecur(Expression const *expr, FILE *stream) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPRESSION_OPERATOR:
            fprintf(stream, "(");
            for (int i = 0; i < expr->nOperands - 1; i ++) {
                expressionToStringRecur(expr->operands[i], stream);
                fprintf(stream, " %c ", expr->op->symbol);
            }

            expressionToStringRecur(expr->operands[expr->nOperands - 1], stream);
            fprintf(stream, ")");
            break;

        case EXPRESSION_DOUBLE:
            if (expr->value < 0) fprintf(stream, "(%g)", expr->value);
            else fprintf(stream, "%g", expr->value);
            break;

        case EXPRESSION_INTEGER:
            if (expr->integer < 0) fprintf(stream, "(%" PRId64 ")", expr->integer);
            else fprintf(stream, "%" PRId64, expr->integer);
            break;

        case EXPRESSION_VARIABLE:
            fprintf(stream, "%s", expr->identifier);
            break;

        case EXPRESSION_FUNCTION_CALL:
            fprintf(stream, "%s(", expr->cmp->identifier);
            for (int i = 0; i < expr->nInputs - 1; i ++) {
                expressionToStringRecur(expr->inputs[i], stream);
                fprintf(stream, ", ");
            }
            expressionToStringRecur(expr->inputs[expr->nInputs - 1], stream);
            fprintf(stream, ")");
            break;

		case EXPRESSION_OBJECT:
			Object const *obj = searchObject(GLOBALCONTEXT->registry, expr->objectId);
			if (obj != NULL && obj->print != NULL) {
				char *out = obj->print(expr->data);
				fprintf(stream, "%s", out);
				free(out);
			}

			else fprintf(stream, "(object)");

			break;
            
        default:
            fprintf(stream, "How'd we get here? %d\n", expr->type);
            break;
    }
}


char *expressionToString(Expression const *expr) {
    FILE *stream = tmpfile();
    char *string = NULL;
    if (expr == NULL || stream == NULL) {
        fclose(stream);
        return string;
    }
    expressionToStringRecur(expr, stream);
    
    long size = ftell(stream);
    if (size < 0) {
        fclose(stream);
        return NULL;
    }

    string = malloc(size + 1);
    if (string == NULL) {
        fclose(stream);
        return NULL;
    }

    rewind(stream);

    size_t ptr = fread(string, 1, size, stream);
    string[ptr] = '\0';

    fclose(stream);
    return string;
}
