#include <stdint.h>
#include <stdlib.h>

#include "simplify.h"
#include "core/common.h"
#include "core/context.h"
#include "core/primitives/numbers.h"
#include "core/utils/expr_utils.h"
#include "core/utils/log.h"

#define DEFAULT_FLATTEN_SIZE 2
#define DEFAULT_FLATTEN_INCREASE 5


static BuiltinResult callImplementations(
		uint32_t nImplementations, BuiltinImplementation const * const implementations,
		Context const *ctx, Expression **exprs, uint32_t nArgs, Expression **out
) {
	if (nImplementations == 0) return BUILTIN_NEUTRAL; 

	BuiltinResult result;
	for (uint32_t i = 0; i < nImplementations; i ++) {
		result = implementations[i](ctx, exprs, nArgs, out);
		if (result == BUILTIN_NEUTRAL) continue;
		break;
	}
	
	return result;
}


static int flattenOpsRecur(Expression *expr, const Operation *op, Expression ***list, int *elts, int *size) {
    if (expr->type == EXPRESSION_OPERATOR && expr->op == op) {
        for (int i = 0; i < expr->nOperands; i ++) {
            if (!flattenOpsRecur(expr->operands[i], op, list, elts, size)) return 0;
        }
        
        // Ensures main flatten function doesnt free expressions moved to the flattened list
        expr->nOperands = 0;
        freeExpression(expr);
        
        return 1;
    }
    
    if (*elts >= *size) {
        *size += DEFAULT_FLATTEN_SIZE;
        Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
        if (temp == NULL) {
            perror("Error flattening operation");
            return 0;
        }
        
        *list = temp;
    }
    
    Debug(0, "\nAdding to list\n");
    Debug(1, printExpression(expr));
    
    (*list)[*elts] = expr;
    (*elts) ++;
    
    return 1;
}


static int flattenOps(Expression *expr) {
    if (expr == NULL) return 1;
    
    int size = DEFAULT_FLATTEN_SIZE;
    int elts = 0;
    Expression **list = malloc(sizeof(Expression *) * size);
    if (list == NULL) {
        perror("Error flattening operation");
        return 0;
    }
    
    Debug(0, "Recursively flattening\n");
    for (int i = 0; i < expr->nOperands; i ++) {
        if (!flattenOpsRecur(expr->operands[i], expr->op, &list, &elts, &size)) {
            free(list);
            return 0;
        }
    }
    
    free(expr->operands);
    
    Debug(0, "\nFlattened list %d\n", elts);
    for (int i = 0; i < elts; i ++) {
        Debug(1, printExpression(list[i]));
    }
    
    expr->operands = list;
    expr->nOperands = elts;
    
    return 1;
}


static bool combineLikeTermsRecur(Expression **ptr) {
    if (*ptr == NULL || (*ptr)->type != EXPRESSION_OPERATOR) return true;
    Expression *expr = *ptr;

	if (expr->type != EXPRESSION_OPERATOR || expr->op->associativity != ASSOC_BOTH) return true; 

	for (uint32_t i = 0; i < expr->nOperands; i ++) {
		for (uint32_t j = i + 1; j < expr->nOperands; j ++) {
			if (expr->operands[i]->type == EXPRESSION_OPERATOR) combineLikeTermsRecur(&expr->operands[i]);
			if (expr->operands[j]->type == EXPRESSION_OPERATOR) combineLikeTermsRecur(&expr->operands[j]);

			if (expr->operands[i]->type != EXPRESSION_OBJECT || expr->operands[j]->type != EXPRESSION_OBJECT) continue;
			if (expr->operands[i]->objectId != expr->operands[j]->objectId) continue;
			if (expr->operands[i]->objectId == NUMBER_ID) {
				Expression *out; 
				Expression *list[2] = {expr->operands[i], expr->operands[j]};

				BuiltinResult result = callImplementations(
						expr->op->nImplementations,
						expr->op->implementations,
						GLOBALCONTEXT, list, 2, &out
				);

				if (result == BUILTIN_ERROR) return false;

				freeExpression(expr->operands[i]);
				freeExpression(expr->operands[j]);
				expr->operands[i] = out;
				expr->operands[j] = expr->operands[expr->nOperands-1];
				expr->nOperands --;
				j --;

				if (i >= expr->nOperands) break;
			}
		}
	}

	if (expr->nOperands == 1) {
		Expression *tmp = expr->operands[0];
		free(expr->operands);
		free(expr);

		*ptr = tmp;
	}

    return true;
}


static bool simplifyRecur(Expression **ptr) {
    if (ptr == NULL || *ptr == NULL) return true;
    if ((*ptr)->type != EXPRESSION_OPERATOR) return true; // No simplification to be done on non-operators
    Expression *expr = *ptr;
    
    Debug(0, "Flattening operation\n");
	if (expr->op->associativity == ASSOC_BOTH && !flattenOps(expr)) return false;
    Debug(0, "\nafter\n");
    Debug(1, printExpression(expr));
    
    for (int i = 0; i < expr->nOperands; i ++) {
        if (!simplifyRecur(&(expr->operands[i]))) return false;
    }
    
    Debug(0, "Combining like terms\n");
    if (!combineLikeTermsRecur(ptr)) return 0;

    return true;
}


bool simplify(Expression **expr) {
    Debug(0, "\nSimplifying\n");
    Debug(1, printExpression(*expr));
    
    if (!simplifyRecur(expr)) return false;
    
    return true;
}
