#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "execute.h"
#include "core/utils/context/context.h"
#include "core/utils/context/environment.h"
#include "core/utils/log.h"
#include "core/utils/types.h"


static int executeRecur(Expression **ptr, Environment *env) {
    Expression *expr = *ptr;
    if (expr == nullptr) return 0;

    Debug(0, "Recursively executing\n");
    Debug(1, printExpression(expr));

    switch (expr->type) {
        case EXPRESSION_INTEGER:
        case EXPRESSION_DOUBLE:
            return 1;

        case EXPRESSION_VARIABLE: {
            Component *cmp = nullptr;
            Environment *curEnv = env;

            // Checks inner to outer environments
            while (cmp == nullptr && curEnv != nullptr) {
                if (expr->type != EXPRESSION_VARIABLE) break;
                cmp = searchEnvironment(curEnv, expr->identifier);

                if (cmp !=  nullptr && cmp->type == COMP_VARIABLE) {
                    Debug(1, printEnvironment(curEnv));
                    free(expr->identifier);
                    free(expr);

                    *ptr = deepCopyExpression(cmp->value);
                    if (!executeRecur(ptr, env)) return 0;

                    return 1;
                }
                curEnv = curEnv->parent;
            }

            return 1;
        }

        case EXPRESSION_OPERATOR:
            for (int i = 0; i < expr->arity; i ++) {
                if (!executeRecur(&(expr->operands[i]), env)) return 0;
            } 

            Debug(0, "Running main operator now\n");
            
            // ==================================================
            // Handle calling operation funciton from environment
            // ==================================================
            
            return 1;

        case EXPRESSION_FUNCTION_CALL:
            FunctionCall *call = expr->call;

            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, call->identifier);
            if (cmp == nullptr || cmp->type == COMP_VARIABLE) {
                printf("Error getting environment component.\n");
                return 0;
            }

            Function *func = cmp->func;
            if (func == nullptr) {
                printf("Function '%s' couldn't be found in the environment.\n", call->identifier);
                return 0;
            }

            if (call->nParams != func->parameters) {
                printf("Expected %d parameters for '%s', %d parameters were passed.\n", func->parameters, call->identifier, call->nParams);
                return 0;
            }

            // Goes up call stack, if global environment is found, then this is evaluating rather than like binding a new variable/function
            bool evaluating = false;
            Environment *tempEnv = env;
            while (tempEnv != nullptr) {
                if (tempEnv == GLOBALCONTEXT->env) { 
                    evaluating = true;
                    break;
                }
                tempEnv = tempEnv->parent;
            }

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return 1;

            if (env != nullptr) func->env->parent = env;
            
            // Updates values of parameters within function environment
            Component *paramCmp = func->env->compList;
            // Counts backwards due to how linked list insertion of environment works, first parameter will come last
            for (int i = func->parameters - 1; i >= 0; i --) {
                freeExpression(paramCmp->value);
                
                if (!executeRecur(&call->parameters[i], env)) {
                    func->env->parent = nullptr;
                    return 0;
                }
                
                paramCmp->value = call->parameters[i];
                paramCmp = paramCmp->next;
            }

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing defined function\n");
                    Debug(1, printEnvironment(func->env));
                    Expression *exec = deepCopyExpression(func->definition);
                    if (exec == nullptr) return 0;
                    if (!executeRecur(&exec, func->env)) {
                        func->env->parent = nullptr;
                        return 0;
                    }

                    func->env->parent = nullptr;
                    call->nParams = 0;
                    freeExpression(expr);

                    Debug(1, printExpression(exec));
                    *ptr = exec;
                    return 1;

                default:
                    printf("Undefined behavior as of now.\n");
            }
            
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