#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "executor.h"
#include "core/common.h"
#include "core/context.h"
#include "simplify.h"

#include "core/utils/expr_utils.h"
#include "core/utils/log.h"
#include "core/context/environment.h"


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


static EXECUTOR_RESULT resolveSymbols(Expression **ptr, Environment *env) {
	Info(0, "Resolving symbols\n");
	if (ptr == NULL || *ptr == NULL) return EXECUTOR_SUCCESS;
	Expression *expr = *ptr;

	EXECUTOR_RESULT result;

	switch (expr->type) {
		case EXPRESSION_VARIABLE:
			Debug(0, "Updating variable '%s'\n", expr->identifier);
			Component const *cmp = searchEnvironment(env, expr->identifier);
			if (cmp != NULL && cmp->type == COMP_VARIABLE) {
				freeExpression(expr);
				*ptr = deepCopyExpression(cmp->value);

				result = resolveSymbols(ptr, env);
				if (result != EXECUTOR_SUCCESS) return result;
				
				return EXECUTOR_SUCCESS;
			}

			Debug(0, "Couldnt resolve symbol %s, leaving as symbolic\n", expr->identifier);

			break;

		case EXPRESSION_FUNCTION_CALL:
			Function const *func = expr->cmp->func;

			// Handles builtin functions
			// Is called now so that the outputs are ready for simplification step
			if (func->type == BUILTIN) {
				for (uint32_t i = 0; i < expr->nInputs; i ++) {
					result = resolveSymbols(&expr->inputs[i], env);
					if (result != EXECUTOR_SUCCESS) return result;

					result = simplify(&expr->inputs[i]);
					if (result != EXECUTOR_SUCCESS) return result;
				}

				BuiltinResult b_result = func->implementation(GLOBALCONTEXT, expr->nInputs, expr->inputs);

				if (b_result.type == BUILTIN_ERROR) return EXECUTOR_ERROR;
				else if (b_result.type == BUILTIN_NEUTRAL) return EXECUTOR_SUCCESS;

				freeExpression(expr);
				*ptr = b_result.output;

				return EXECUTOR_SUCCESS;
			}

			// For user defined functions, can resolve identifier into the expression
			// tree from environment
			Environment *tmpEnv = createEnvironment();
			if (tmpEnv == NULL) {
				perror("Error resolving symbols");
				return EXECUTOR_ERROR;
			}
			tmpEnv->parent = env;

			for (uint32_t i = 0; i < expr->nInputs; i ++) {
				result = resolveSymbols(&expr->inputs[i], env);
				if (result != EXECUTOR_SUCCESS) return result;

				if (!bindComponent(tmpEnv, COMP_VARIABLE, func->parameters[i], expr->inputs[i])) return EXECUTOR_ERROR;
			}

			freeExpression(expr);
			*ptr = deepCopyExpression(func->definition);
			Debug(1, printExpression(*ptr));
			result = resolveSymbols(ptr, tmpEnv);
			if (result != EXECUTOR_SUCCESS) return result;

			Debug(1, printExpression(*ptr));
			freeEnvironment(tmpEnv);

			break;

		case EXPRESSION_OPERATOR:
			for (uint32_t i = 0; i < expr->nOperands; i ++) {
				result = resolveSymbols(&expr->operands[i], env);
				if (result != EXECUTOR_SUCCESS) return result;
			}

			break;

		default:
			break;
	}

	return EXECUTOR_SUCCESS;
}


EXECUTOR_RESULT execute(Expression **ptr, char **output) {
	EXECUTOR_RESULT result;
	result = resolveSymbols(ptr, GLOBALCONTEXT->env);
	if (result != EXECUTOR_SUCCESS) goto error;

	if (!simplify(ptr)) goto error;

	*output = expressionToString(*ptr);

	return EXECUTOR_SUCCESS;

	error: 
		freeExpression(*ptr);
		return result;
}
