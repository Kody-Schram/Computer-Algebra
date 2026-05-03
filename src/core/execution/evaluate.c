#include <stdio.h>
#include <stdint.h>

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
        case EXPRESSION_INTEGER:
        case EXPRESSION_DOUBLE:
            return true;

        case EXPRESSION_VARIABLE: {
            Debug(0, "Updating variable '%s'\n", expr->identifier);
            Component *cmp = NULL;
            Environment *curEnv = env;

            // Checks inner to outer environments
            while (cmp == NULL && curEnv != NULL) {
                if (expr->type != EXPRESSION_VARIABLE) break;
                cmp = searchEnvironment(curEnv, expr->identifier);

                if (cmp !=  NULL && cmp->type == COMP_VARIABLE) {
                    Debug(1, printEnvironment(curEnv));
                    freeExpression(expr);

                    *ptr = deepCopyExpression(cmp->value);
                    if (!evaluateRecur(ptr, env)) return false;

                    return true;
                }
                curEnv = curEnv->parent;
            }

            return true;
        }

        case EXPRESSION_OPERATOR:
            Debug(0, "\nOperands %d\n", expr->nOperands);
            Debug(1, printExpression(expr));
            
            for (uint32_t i = 0; i < expr->nOperands; i ++) {
                if (!evaluateRecur(&(expr->operands[i]), env)) return false;
            }
        
            if (expr->nOperands != expr->op->arity) return true; // Unexpected number of operands, leave symbolic
			BuiltinResult result = callImplementations(
									expr->op->nImplementations, 
									expr->op->implementations,
									expr->nOperands,
									expr->operands
									);

			if (result.type == BUILTIN_ERROR) return false;
			else if (result.type == BUILTIN_NEUTRAL) return true;

			freeExpression(expr);
			*ptr = result.output;
			return true;
 

        case EXPRESSION_FUNCTION_CALL:
            FunctionCall *call = expr->call;

            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, call->identifier);
            if (cmp == NULL || cmp->type == COMP_VARIABLE) {
                printf("Error getting environment component.\n");
                return false;
            }

            Function *func = cmp->func;
            if (func == NULL) {
                printf("Function '%s' couldn't be found in the environment.\n", call->identifier);
                return false;
            }

            if (call->nParams != func->nParameters) {
                printf("Expected %d parameters for '%s', %d parameters were passed.\n", func->nParameters, call->identifier, call->nParams);
                return false;
            }

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

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return true;
           
			if (func->type == DEFINED) {
				Environment *callEnv = createEnvironment(ENV_LIST);
				if (callEnv == NULL) return false;
				
				callEnv->parent = env;
				
				for (uint32_t i = 0; i < func->nParameters; i ++) {
					if (!evaluateRecur(&call->parameters[i], env)) {
						freeEnvironment(callEnv);
						return false;
					}
					
					if (!bindComponent(callEnv, COMP_VARIABLE, func->parameters[i], call->parameters[i])) {
						freeEnvironment(callEnv);
						return false;
					}
				}
				
				Debug(0, "Call env\n");
				Debug(1, printEnvironment(callEnv));
				
				// Prevents double free of params
				expr->call->nParams = 0;

				Debug(0, "Executing defined function\n");
				Expression *exec = deepCopyExpression(func->definition);
				if (exec == NULL) return false;
				if (!evaluateRecur(&exec, callEnv)) {
					freeEnvironment(callEnv);
					return false;
				}
				
				freeExpression(expr);

				Debug(0, "\nCall result\n");
				Debug(1, printExpression(exec));
				*ptr = exec;

				freeEnvironment(callEnv);
				return true;
            }

			for (uint32_t i = 0; i < func->nParameters; i ++) {
				if (!evaluateRecur(&call->parameters[i], env)) return false;
			}

			result = callImplementations(
									func->nImplementations,
									func->implementations,
									func->nParameters,
									call->parameters
									);

			if (result.type == BUILTIN_ERROR) return false;
			else if (result.type == BUILTIN_NEUTRAL) return true;

			freeExpression(expr);
			*ptr = result.output;
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
