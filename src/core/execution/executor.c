#include <stdint.h>
#include <inttypes.h>

#include "executor.h"
#include "core/common.h"
#include "core/context.h"
#include "simplify.h"

#include "core/utils/expr_utils.h"
#include "core/utils/log.h"
#include "core/context/environment.h"


static EXECUTOR_RESULT resolveSymbols(Expression **ptr, Environment *env, bool isExecuting) {
	if (ptr == NULL || *ptr == NULL) return EXECUTOR_SUCCESS;
	if (!isExecuting && GLOBALCONTEXT->config->LAZY_RESOLUTIONS) return EXECUTOR_SUCCESS;

	Expression *expr = *ptr;

	EXECUTOR_RESULT result;

	switch (expr->type) {
		case EXPRESSION_VARIABLE:
			Debug(0, "Trying to resolve variable '%s'\n", expr->identifier);
			Component const *cmp = searchEnvironment(env, expr->identifier);
			if (cmp != NULL && cmp->type == COMP_VARIABLE) {
				freeExpression(expr);
				*ptr = deepCopyExpression(cmp->value);

				result = resolveSymbols(ptr, env, isExecuting);
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
					result = resolveSymbols(&expr->inputs[i], env, isExecuting);
					if (result != EXECUTOR_SUCCESS) return result;

					result = simplify(&expr->inputs[i]);
					if (result != EXECUTOR_SUCCESS) return result;
				}
				
				Expression *out;
				BuiltinResult b_result = func->implementation(GLOBALCONTEXT, expr->inputs, expr->nInputs, &out);

				if (b_result == BUILTIN_ERROR) return EXECUTOR_ERROR;
				else if (b_result == BUILTIN_NEUTRAL) return EXECUTOR_SUCCESS;

				freeExpression(expr);
				*ptr = out;

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
				result = resolveSymbols(&expr->inputs[i], env, isExecuting);
				if (result != EXECUTOR_SUCCESS) return result;

				if (!bindComponent(tmpEnv, COMP_VARIABLE, func->parameters[i], expr->inputs[i])) return EXECUTOR_ERROR;
			}

			freeExpression(expr);
			*ptr = deepCopyExpression(func->definition);
			Debug(1, printExpression(*ptr));
			result = resolveSymbols(ptr, tmpEnv, isExecuting);
			freeEnvironment(tmpEnv);
			if (result != EXECUTOR_SUCCESS) return result;

			Debug(1, printExpression(*ptr));

			break;

		case EXPRESSION_OPERATOR:
			for (uint32_t i = 0; i < expr->nOperands; i ++) {
				if (expr->operands[i]->type != EXPRESSION_OPERATOR &&
					expr->operands[i]->type != EXPRESSION_VARIABLE &&
					expr->operands[i]->type != EXPRESSION_FUNCTION_CALL) continue;

				result = resolveSymbols(&expr->operands[i], env, isExecuting);
				if (result != EXECUTOR_SUCCESS) return result;
			}

			break;

		default:
			break;
	}

	return EXECUTOR_SUCCESS;
}


EXECUTOR_RESULT execute(Expression **ptr, bool isExecuting) {
	EXECUTOR_RESULT result;
	Info(0, "Resolving symbols\n");
	result = resolveSymbols(ptr, GLOBALCONTEXT->env, isExecuting);
	if (result != EXECUTOR_SUCCESS) goto error;

	if (!simplify(ptr)) goto error;

	return EXECUTOR_SUCCESS;

	error: 
		freeExpression(*ptr);
		return result;
}
