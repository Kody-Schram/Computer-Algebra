#include <stdio.h>

#include "execute.h"
#include "core/context/context.h"
#include "core/context/environment.h"
#include "core/utils/log.h"
#include "core/primitives/types.h"
#include "core/utils/type_utils.h"


static int executeRecur(Expression **ptr, Environment *env) {
    if (ptr == NULL || *ptr == NULL) return 0;
    Expression *expr = *ptr;

    Debug(0, "Recursively executing\n");
    Debug(1, printExpression(expr));

    switch (expr->type) {
        case EXPRESSION_INTEGER:
        case EXPRESSION_DOUBLE:
            return 1;

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
                    if (!executeRecur(ptr, env)) return 0;

                    return 1;
                }
                curEnv = curEnv->parent;
            }

            return 1;
        }

        case EXPRESSION_OPERATOR:
            if (expr->nOperands != expr->op->arity) return 1; // Unexpected number of operands, leave symbolic
            
            for (int i = 0; i < expr->nOperands; i ++) {
                if (!executeRecur(&(expr->operands[i]), env)) return 0;
            }
        
            for (int i = 0; i < expr->op->nDefs; i ++) {
                Function *func = expr->op->definitions[i];
                Debug(0, "Running main operator now\n");
                
                BuiltinResult result = func->builtin(expr->nOperands, expr->operands);
                if (result.type == BUILTIN_ERROR) return 0;
                if (result.type == BUILTIN_NEUTRAL) continue; // keeps running until one of the definitions evaluates or errors
                
                Debug(0, "Output of operation\n");
                Debug(1, printExpression(result.output));
                
                if (result.output == NULL) return 1;
                
                freeExpression(expr);
                *ptr = result.output;
                return 1;
            }
                    
            return 1;

        case EXPRESSION_FUNCTION_CALL:
            FunctionCall *call = expr->call;

            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, call->identifier);
            if (cmp == NULL || cmp->type == COMP_VARIABLE) {
                printf("Error getting environment component.\n");
                return 0;
            }

            Function *func = cmp->func;
            if (func == NULL) {
                printf("Function '%s' couldn't be found in the environment.\n", call->identifier);
                return 0;
            }

            if (call->nParams != func->nParameters) {
                printf("Expected %d parameters for '%s', %d parameters were passed.\n", func->nParameters, call->identifier, call->nParams);
                return 0;
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

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return 1;
            
            Environment *callEnv = createEnvironment(ENV_LIST);
            if (callEnv == NULL) return 0;
            
            callEnv->parent = env;
            
            for (int i = 0; i < func->nParameters; i ++) {
                if (!executeRecur(&call->parameters[i], env)) {
                    freeEnvironment(callEnv);
                    return 0;
                }
                
                if (!bindComponent(callEnv, COMP_VARIABLE, func->parameters[i], call->parameters[i])) {
                    freeEnvironment(callEnv);
                    return 0;
                }
            }
            
            Debug(0, "Call env\n");
            Debug(1, printEnvironment(callEnv));
            
            // Prevents double free of params
            expr->call->nParams = 0;

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing defined function\n");
                    Expression *exec = deepCopyExpression(func->definition);
                    if (exec == NULL) return 0;
                    if (!executeRecur(&exec, callEnv)) {
                        freeEnvironment(callEnv);
                        return 0;
                    }
                    
                    freeExpression(expr);

                    Debug(0, "\nCall result\n");
                    Debug(1, printExpression(exec));
                    *ptr = exec;
                    break;
 
                default:
                    printf("Undefined behavior as of now.\n");
            }
            
            freeEnvironment(callEnv);
            
            return 1;
    }

    return 1;
}


int execute(Expression **expr) {
    Info(0, "\nExecuting\n");
    Info(1, printExpression(*expr));
    if (!executeRecur(expr, GLOBALCONTEXT->env)) return 0;

    return 1;
}