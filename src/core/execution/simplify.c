#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "simplify.h"
#include "core/common.h"
#include "core/context.h"
#include "core/primitives/numbers.h"
#include "core/utils/expr_utils.h"
#include "core/utils/log.h"

#define DEFAULT_FLATTEN_SIZE 2
#define DEFAULT_FLATTEN_INCREASE 5


static bool flattenOpsRecur(Expression *expr, Operation const *op, Expression ***list, int *elts, int *size) {
    if (expr->type == EXPRESSION_OPERATOR && expr->op == op) {
        for (int i = 0; i < expr->nOperands; i ++) {
            if (!flattenOpsRecur(expr->operands[i], op, list, elts, size)) return false;
        }
        
        // Ensures main flatten function doesnt free expressions moved to the flattened list
        expr->nOperands = 0;
        freeExpression(expr);
        
        return true;
    }
    
    if (*elts >= *size) {
        *size += DEFAULT_FLATTEN_SIZE;
        Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
        if (temp == NULL) {
            perror("Error flattening operation");
            return false;
        }
        
        *list = temp;
    }
    
    Debug(0, "\nAdding to list\n");
    Debug(1, printExpression(expr));
    
    (*list)[*elts] = expr;
    (*elts) ++;
    
    return true;
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
				Expression *new = dummyExpression(EXPRESSION_OBJECT); 
					if (new == NULL) return false;

				BuiltinResult result;
				ObjectData a = {
					.id = expr->operands[0]->objectId,
					.value = expr->operands[0]->value,
					.flags = expr->operands[0]->flags
				};

				ObjectData b = {
					.id = expr->operands[1]->objectId,
					.value = expr->operands[1]->value,
					.flags = expr->operands[1]->flags
				};

				ObjectData out = {
					.id = NUMBER_ID,
					.value.integer = 0,
					.flags = 0
				};

				result = dispatchOperation(expr->op, a, b, &out);

				if (result == BUILTIN_ERROR) return false;

				new->objectId = out.id;
				new->value = out.value;
				new->flags = out.flags;

				freeExpression(expr->operands[i]);
				freeExpression(expr->operands[j]);
				expr->operands[i] = new;
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


bool collapseRecur(Expression **ptr, Operation const *op, Expression ***list, uint32_t *size, uint32_t *elts);


static inline bool collapseOperation(Expression **ptr, Operation const *op, Expression ***list, uint32_t *size, uint32_t *elts) {
	if (list != NULL && op == (*ptr)->op) {
		if (!collapseRecur(ptr, op, list, size, elts)) return false;
		return true;
	}

	uint32_t newSize = 2;
	uint32_t newElts = 0;
	Expression **newList = malloc(sizeof(Expression *) * newSize);
	if (newList == NULL) return false;

	if (!collapseRecur(ptr, (*ptr)->op, &newList, &newSize, &newElts)) {
		free(newList);
		return false;
	}

	if (newElts == 1) {
		*ptr = newList[0];
		free(newList);
		return true;
	}

	free((*ptr)->operands);
	(*ptr)->operands = newList;
	(*ptr)->nOperands = newElts;

	return true;
}


bool collapseRecur(Expression **ptr, Operation const *op, Expression ***list, uint32_t *size, uint32_t *elts) {
	Expression *expr = *ptr;
	if (expr->type == EXPRESSION_FUNCTION_CALL) {
		for (uint32_t i = 0; i < expr->nInputs; i ++) {
			if (expr->inputs[i]->type != EXPRESSION_OPERATOR && expr->inputs[i]->type != EXPRESSION_FUNCTION_CALL) continue;

			if (expr->inputs[i]->type == EXPRESSION_OPERATOR && 
					!collapseOperation(&expr->inputs[i], NULL, NULL, NULL, NULL)) return false;
			else if (!collapseRecur(&expr->inputs[i], NULL, NULL, NULL, NULL)) return false;
		}

		if (list == NULL) return true;

		if (*elts >= *size) {
			*size += 1;
			Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
			if (temp == NULL) {
				perror("Error flattening operation");
				return false;
			}
			
			*list = temp;
		}
		
		(*list)[*elts] = expr;
		(*elts) ++;

		return true;
	}


	if (expr->operands[0]->type == EXPRESSION_OPERATOR &&
			!collapseOperation(&expr->operands[0], op, list, size, elts)) return false;
	if (expr->operands[1]->type == EXPRESSION_OPERATOR &&
			!collapseOperation(&expr->operands[1], op, list, size, elts)) return false;

	// No flattening has been done yet, should only be 2 operands
	Expression *a = expr->operands[0];
	Expression *b = expr->operands[1];

	if (a->type == EXPRESSION_OBJECT && b->type == EXPRESSION_OBJECT) {

		// Applies constant folding
		if ((a->objectId == NUMBER_ID && b->objectId == NUMBER_ID) || (IS_CONSTANT(a->flags) && IS_CONSTANT(b->flags))) {
			BuiltinResult result;
			ObjectData x = {
				.id = a->objectId,
				.value = a->value,
				.flags = a->flags
			};

			ObjectData y = {
				.id = b->objectId,
				.value = b->value,
				.flags = b->flags
			};

			ObjectData out = {
				.id = NUMBER_ID,
				.flags = 0
			};

			result = dispatchOperation(expr->op, x, y, &out);

			if (result == BUILTIN_ERROR) return false;

			Expression *new = dummyExpression(EXPRESSION_OBJECT);
			if (new == NULL) return false;

			new->objectId = out.id;
			new->flags = out.flags;
			new->value = out.value;

			// This will also free the child a and b nodes
			freeExpression(expr);

			if (list == NULL) {
				*ptr = new;
				return true;
			}

			if (*elts >= *size) {
				*size += 1;
				Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
				if (temp == NULL) {
					perror("Error flattening operation");
					return false;
				}
				
				*list = temp;
			}

			(*list)[(*elts)] = new; 
			(*elts) ++;

			return true;
		}
	}

	if (list == NULL) return true;

	// Appends children to list
	if (*elts + 1 >= *size) {
		*size += 2;
		Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
		if (temp == NULL) {
			perror("Error flattening operation");
			return false;
		}
		
		*list = temp;
	}
	
	(*list)[*elts] = a;
	(*elts) ++;
	(*list)[*elts] = b;
	(*elts) ++;

	return true;
}




bool simplify(Expression **ptr) {
    Debug(0, "\nSimplifying\n");
    Debug(1, printExpression(*ptr));
    
    //if (!simplifyRecur(expr)) return false;
	
	if ((*ptr)->type != EXPRESSION_OPERATOR && (*ptr)->type != EXPRESSION_FUNCTION_CALL) return true;
	/*
	if ((*ptr)->type == EXPRESSION_OPERATOR) return collapseOperation(ptr, NULL, NULL, NULL, NULL);
	return collapseRecur(ptr, NULL, NULL, NULL, NULL);
	*/
	return true;
}
