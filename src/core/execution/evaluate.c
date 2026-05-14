#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "evaluate.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"
#include "core/execution/execution_utils.h"


static bool evaluateRecur(Expression **ptr, Environment *env) {
    if (ptr == NULL || *ptr == NULL) return false;
    Expression *expr = *ptr;

    Debug(0, "Recursively executing\n");
    Debug(1, printExpression(expr));

    switch (expr->type) {
        case EXPRESSION_OPERATOR:
            Debug(0, "\nOperands %d\n", expr->nOperands);
            Debug(1, printExpression(expr));

			bool valid = true;
            for (uint32_t i = 0; i < expr->nOperands; i ++) {
                if (!evaluateRecur(&(expr->operands[i]), env)) return false;
				if (expr->operands[i]->type != EXPRESSION_OBJECT) valid = false;
            }

			// Unexpected number of operands, leave symbolic
            if (!valid || expr->nOperands != expr->op->arity) return true; 			BuiltinResult result = callImplementations(
									expr->op->nImplementations, 
									expr->op->implementations,
									GLOBALCONTEXT,
									expr->nOperands,
									expr->operands
									);

			if (result.type == BUILTIN_ERROR) return false;
			else if (result.type == BUILTIN_NEUTRAL) return true;

			freeExpression(expr);
			*ptr = result.output;
			return true;
 

        case EXPRESSION_FUNCTION_CALL:
			const Function *func = expr->cmp->func;
            if (expr->nInputs != func->nParameters) {
                printf("Expected %" PRIu32 " parameters for '%s', %" PRIu32 " parameters were passed.\n", func->nParameters, expr->cmp->identifier, expr->nInputs);
                return false;
            }

			if (GLOBALCONTEXT->config->LAZY_CALLS) return true;

            // Goes up call stack, if global environment is found, then this is evaluating rather than like binding a new variable/function
            bool evaluating = false;
            Environment *tempEnv = env;
            while (tempEnv != NULL) {
                if (tempEnv == GLOBALCONTEXT->env) { 
                    evaluating = true;
                    break;
                }
                tempEnv = tempEnv->parent;
            }

            if (!evaluating) return true;
           

			for (uint32_t i = 0; i < expr->nInputs; i ++) {
				if (!evaluateRecur(&expr->inputs[i], env)) return false;
			}

			result = callImplementations(
									func->nImplementations,
									func->implementations,
									GLOBALCONTEXT,
									expr->nInputs,	
									expr->inputs
									);

			if (result.type == BUILTIN_ERROR) return false;
			else if (result.type == BUILTIN_NEUTRAL) return true;

			freeExpression(expr);
			*ptr = result.output;
			return true;

		default:
			return true;
    }


    return true;
}


bool evaluate(Expression **expr) {
    Info(0, "\nExecuting\n");
    Info(1, printExpression(*expr));
    if (!evaluateRecur(expr, GLOBALCONTEXT->env)) return false;

    return true;
}
