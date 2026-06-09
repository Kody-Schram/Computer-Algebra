#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simplify.h"
#include "core/common.h"
#include "core/context.h"
#include "core/primitives/numbers.h"
#include "core/utils/expr_utils.h"
#include "core/utils/log.h"

#define DEFAULT_FLATTEN_SIZE 2
#define DEFAULT_FLATTEN_INCREASE 5


static inline bool foldConstants(Operation const *op, Expression *a, Expression *b, Expression **expr_out) {
	if (a->type == EXPRESSION_OBJECT && b->type == EXPRESSION_OBJECT) {

		// Applies constant folding
		if ((a->objectId == NUMBER_ID && b->objectId == NUMBER_ID) || (IS_CONSTANT(a->flags) && IS_CONSTANT(b->flags))) {
			BuiltinResult result;

			ObjectData x = BUILD_OBJECT_DATA(a);
			ObjectData y = BUILD_OBJECT_DATA(b);

			ObjectData out = {
				.value = 0,
				.flags = 0,
				.meta = 0
			};

			uint64_t id;

			result = dispatchOperation(op, a->objectId, x, b->objectId, y, &out, &id);

			if (result == BUILTIN_ERROR) return false;

			Expression *new = dummyExpression(EXPRESSION_OBJECT);
			if (new == NULL) return false;

			new->objectId = id;
			new->meta = out.meta;
			new->flags = out.flags;
			new->value = out.value;

			Object const *obj = searchObject(GLOBALCONTEXT->registry, id);
			char *str = obj->print(out);
			Debug(0, "value: %s\n", str); 
			free(str);

			Debug(0, "Dispatch output:\n");
			Debug(1, printExpression(new));

			*expr_out = new;

			return true;
		}
	}

	return true;
}


bool collapse(Expression **ptr, Operation const *op, Expression ***list, uint32_t *size, uint32_t *elts) {
	// Handles each input
	if ((*ptr)->type == EXPRESSION_FUNCTION_CALL) {
		for (uint32_t i = 0; i < (*ptr)->nInputs; i ++) {
			if (((*ptr)->inputs[i]->type == EXPRESSION_OPERATOR || (*ptr)->inputs[i]->type == EXPRESSION_FUNCTION_CALL) &&
					!collapse(&(*ptr)->inputs[i], NULL, NULL, NULL, NULL)) return false;
		}

		return true;
	}

	// Existing list for the same operation, then fold into that list
	if (list != NULL && (*ptr)->op == op) {
		if (((*ptr)->operands[0]->type == EXPRESSION_OPERATOR || (*ptr)->operands[0]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[0], op, list, size, elts)) return false;
		if (((*ptr)->operands[1]->type == EXPRESSION_OPERATOR || (*ptr)->operands[1]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[1], op, list, size, elts)) return false;

		Expression *out = NULL;
		if ((*ptr)->operands[0] != NULL && (*ptr)->operands[1] != NULL &&
				!foldConstants((*ptr)->op, (*ptr)->operands[0], (*ptr)->operands[1], &out)) return false;

		// If collapses to constant, then swap in place of ptr and have parent node handle adding to list 
		if (out != NULL) {
			freeExpression(*ptr);
			*ptr = out;
			
			return true;
		}

		// Otherwise, add children
		if (*elts + 1 >= *size) {
			*size *= 2;
			Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
			if (temp == NULL) {
				perror("Error flattening operation");
				return false;
			}
			
			*list = temp;
		}
		
		if ((*ptr)->operands[0] != NULL) {
			(*list)[*elts] = (*ptr)->operands[0];
			(*elts) ++;
		}
		if ((*ptr)->operands[1] != NULL) {
			(*list)[*elts] = (*ptr)->operands[1];
			(*elts) ++;
		}

		free((*ptr)->operands);
		free(*ptr);
		*ptr = NULL;

		return true;
	}

	// Not associative, only try to fold constants, no flattening
	if ((*ptr)->op->associativity != ASSOC_BOTH) {
		if (((*ptr)->operands[0]->type == EXPRESSION_OPERATOR || (*ptr)->operands[0]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[0], NULL, NULL, NULL, NULL)) return false;
		if (((*ptr)->operands[1]->type == EXPRESSION_OPERATOR || (*ptr)->operands[1]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[1], NULL, NULL, NULL, NULL)) return false;

		Expression *out = NULL;
		if ((*ptr)->operands[0] != NULL && (*ptr)->operands[1] != NULL &&
				!foldConstants((*ptr)->op, (*ptr)->operands[0], (*ptr)->operands[1], &out)) return false;

		if (out != NULL) {
			freeExpression(*ptr);
			*ptr = out;
		}

		if (list != NULL) {
			if (*elts >= *size) {
				*size *= 2;
				Expression **temp = realloc(*list, sizeof(Expression*) * (*size));
				if (temp == NULL) {
					perror("Error flattening operation");
					return false;
				}
				
				*list = temp;
			}
			
			(*list)[*elts] = *ptr;
			(*elts) ++;
		}

		return true;
	}


	// Case where no list is established or mismatched operators
	if (list == NULL || (*ptr)->op != op) {
		uint32_t newSize = 2;
		uint32_t newElts = 0;
		Expression **newList = malloc(sizeof(Expression *) * newSize);
		if (newList == NULL) return false;

		if (((*ptr)->operands[0]->type == EXPRESSION_OPERATOR || (*ptr)->operands[0]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[0], (*ptr)->op, &newList, &newSize, &newElts)) return false;
		if (((*ptr)->operands[1]->type == EXPRESSION_OPERATOR || (*ptr)->operands[1]->type == EXPRESSION_FUNCTION_CALL) &&
				!collapse(&(*ptr)->operands[1], (*ptr)->op, &newList, &newSize, &newElts)) return false;

		Expression *out = NULL;
		if ((*ptr)->operands[0] != NULL && (*ptr)->operands[1] != NULL &&
				!foldConstants((*ptr)->op, (*ptr)->operands[0], (*ptr)->operands[1], &out)) return false;

		if (out != NULL) {
			free(newList);
			freeExpression(*ptr);
			*ptr = out;

			return true;
		}

		if ((*ptr)->operands[0] != NULL) {
			if (newElts >= newSize) {
				newSize *= 2;
				Expression **temp = realloc(newList, sizeof(Expression*) * (newSize));
				if (temp == NULL) {
					perror("Error flattening operation");
					return false;
				}
				
				newList = temp;
			}
			
			newList[newElts] = (*ptr)->operands[0];
			newElts ++;
		}

		if ((*ptr)->operands[1] != NULL) {
			if (newElts >= newSize) {
				newSize *= 2;
				Expression **temp = realloc(newList, sizeof(Expression*) * (newSize));
				if (temp == NULL) {
					perror("Error flattening operation");
					return false;
				}
				
				newList = temp;
			}
			
			newList[newElts] = (*ptr)->operands[1];
			newElts ++;
		}


		free((*ptr)->operands);
		(*ptr)->operands = newList;
		(*ptr)->nOperands = newElts;

		return true;
	}

	return true;
}


bool simplify(Expression **ptr) {
    Debug(0, "\nSimplifying\n");
    Debug(1, printExpression(*ptr));

	char *out = expressionToString(*ptr);

    printf("%s\n\n", out);
	free(out);
    
    //if (!simplifyRecur(expr)) return false;
	
	if ((*ptr)->type != EXPRESSION_OPERATOR && (*ptr)->type != EXPRESSION_FUNCTION_CALL) return true;
	return collapse(ptr, NULL, NULL, NULL, NULL);
	//return collapseRecur(ptr, NULL, NULL, NULL, NULL);
}
