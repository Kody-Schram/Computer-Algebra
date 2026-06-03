#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "evaluate.h"
#include "core/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"


static BuiltinResult callImplementations(
		uint32_t nImplementations, BuiltinImplementation const * const implementations,
		Context const *ctx, uint32_t nArgs, Expression **exprs
) {
	if (nImplementations == 0) return (BuiltinResult) {.type = BUILTIN_NEUTRAL, .output=NULL };

	BuiltinResult result;
	for (uint32_t i = 0; i < nImplementations; i ++) {
		result = implementations[i](ctx, nArgs, exprs);
		if (result.type == BUILTIN_NEUTRAL) continue;
		break;
	}
	
	return result;
}


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
            if (!valid || expr->nOperands != expr->op->arity) return true; 			
			BuiltinResult result = callImplementations(
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
			printf("running evaluate call handler\n");
			const Function *func = expr->cmp->func;


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
