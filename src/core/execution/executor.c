#include <stdint.h>
#include <inttypes.h>

#include "executor.h"
#include "core/context/context.h"
#include "core/primitives/types.h"
#include "simplify.h"
#include "evaluate.h"

#include "core/utils/type_utils.h"
#include "core/utils/log.h"
#include "core/context/environment.h"


static bool resolveSymbols(Expression **ptr, Environment *env) {
	Info(0, "Resolving symbols\n");
	if (ptr == NULL || *ptr == NULL) return true;
	Expression *expr = *ptr;

	switch (expr->type) {
		case EXPRESSION_VARIABLE:
			Debug(0, "Updating variable '%s'\n", expr->identifier);
			Component *cmp = NULL;
			const Environment *curEnv = env;

			// Checks inner to outer environments
			while (cmp == NULL && curEnv != NULL) {
				if (expr->type != EXPRESSION_VARIABLE) break;
				cmp = searchEnvironment(curEnv, expr->identifier);

				if (cmp !=  NULL && cmp->type == COMP_VARIABLE) {
					Debug(1, printEnvironment(curEnv));
					freeExpression(expr);

					*ptr = deepCopyExpression(cmp->value);
					return true;
				}
				curEnv = curEnv->parent;
			}

			Debug(0, "Couldnt resolve symbol %s, leaving as symbolic\n", expr->identifier);
			return true;

		case EXPRESSION_FUNCTION_CALL:
			const Function *func = expr->cmp->func;

			if (expr->nInputs != func->nParameters) {
                printf(
					"Expected %" PRIu32 " parameters for '%s', %" PRIu32 " parameters were passed.\n", 
					func->nParameters, expr->cmp->identifier, expr->nInputs
				);
                return false;
			}

			// For builtin functions, resolutions happen to parameters, but the call remains intact
			// until the evaluator actually calls the function
			if (func->type == BUILTIN) {
				for (uint32_t i = 0; i < expr->nInputs; i ++) {
					if (!resolveSymbols(&expr->inputs[i], env)) return false;
				}

				return true;
			}

			// For user defined functions, can resolve identifier into the expression
			// tree from environment
			Environment *tmpEnv = createEnvironment(ENV_LIST);
			if (tmpEnv == NULL) {
				perror("Error resolving symbols");
				return false;
			}
			tmpEnv->parent = env;

			for (uint32_t i = 0; i < expr->nInputs; i ++) {
				if (!resolveSymbols(&expr->inputs[i], env)) return false;
				if (!bindComponent(
						tmpEnv, COMP_VARIABLE, func->parameters[i], expr->inputs[i]
				)) return false;
			}

			freeExpression(expr);
			*ptr = deepCopyExpression(func->definition);
			Debug(1, printExpression(*ptr));
			if (!resolveSymbols(ptr, tmpEnv)) return false;

			Debug(1, printExpression(*ptr));
			freeEnvironment(tmpEnv);
			return true;

		case EXPRESSION_OPERATOR:
			for (uint32_t i = 0; i < expr->nOperands; i ++) {
				if (!resolveSymbols(&expr->operands[i], env)) return false;
			}

			return true;

		default:
			return true;


	}

	return true;
}


bool execute(Expression **ptr, char **output) {
	if (!resolveSymbols(ptr, GLOBALCONTEXT->env)) return false;
	Debug(1, printExpression(*ptr));

	if (!simplify(ptr)) return false;
	if (!evaluate(ptr)) return false;

	*output = expressionToString(*ptr);

	return true;
}
