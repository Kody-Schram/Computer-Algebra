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
            int evaluating = 0;
            Environment *tempEnv = env;
            while (tempEnv != nullptr) {
                if (tempEnv == GLOBALCONTEXT->env) { 
                    evaluating = 1;
                    break;
                }
                tempEnv = tempEnv->parent;
            }

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return 1;

            Environment *localEnv = createEnvironment(ENV_LIST);
            if (localEnv == nullptr) return 0;
            if (env != nullptr) localEnv->parent = env;
            
            // Copy parameter identifiers before freeing function environment
            Component *parameterCmp = func->env->compList;
            char **parameterIdentifiers = malloc(sizeof(char *) * func->parameters);
            if (parameterIdentifiers == nullptr) {
                perror("Error in executing function call");
                freeEnvironment(localEnv);
                return 0;
            }
            
            for (int i = 0; i < func->parameters; i ++) {
                parameterIdentifiers[i] = strdup(parameterCmp->identifier);
                
                if (parameterIdentifiers[i] == nullptr) {
                    perror("Error in executing function call");
                    freeEnvironment(localEnv);
                    for (int j = 0; j < i; j ++) {
                        free(parameterIdentifiers[j]);
                    }
                    free(parameterIdentifiers);
                    return 0;
                }
            }
            
            // Replace functions local environment with the new values
            freeEnvironment(func->env);
            for (int p = 0; p < func->parameters; p ++) {
                // Updates local environment variable definitions with copies of passed in parameters
                if (!executeRecur(&call->parameters[p], localEnv) ) {
                    freeEnvironment(localEnv);
                    return 0;
                }
                if (!bindComponent(localEnv, COMP_VARIABLE, parameterIdentifiers[p], call->parameters[p])) {
                    freeEnvironment(localEnv);
                    return 0;
                }

                // Updates parameters with outer variables
                if (!executeRecur(&localEnv->compList[p].value, env)) {
                    freeEnvironment(localEnv);
                    return 0;
                }
            }
            func->env = localEnv;

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing defined function\n");
                    Debug(1, printEnvironment(localEnv));
                    Expression *exec = deepCopyExpression(func->definition);
                    if (exec == nullptr) return 0;
                    if (!executeRecur(&exec, func->env)) {
                        freeEnvironment(localEnv);
                        return 0;
                    }

                    localEnv->parent = nullptr;
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