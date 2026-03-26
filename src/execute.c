#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "execute.h"
#include "utils/types.h"
#include "utils/context/context.h"
#include "utils/log.h"

static const int DEFAULT_ENV_STACK = 3;

static int replace(ASTNode **ptr, Environment *env);
static int executeRecur(ASTNode **ptr, Environment *env);


static int replace(ASTNode **ptr, Environment *env) {
    ASTNode *ast = *ptr;
    if (ast == NULL || env == NULL) return 1;

    switch (ast->type) {
        case NODE_VARIABLE:
            Component *cmp = NULL;
            Environment *curEnv = env;

            // Checks inner to outer environments
            while (cmp == NULL && curEnv != NULL) {
                if (ast->type != NODE_VARIABLE) break;
                cmp = searchEnvironment(curEnv, ast->identifier);

                if (cmp !=  NULL && cmp->type == VARIABLE) {
                    Debug(0, "Replacing '%s' with recursive definition\n", ast->identifier);
                    free(ast->identifier);
                    free(ast);

                    ASTNode *temp = deepCopyAST(cmp->value);
                    if (temp == NULL) return 0;
                    if (!replace(&temp, env)) {
                        freeAST(temp);
                        return 0;
                    }

                    *ptr = temp;
                    Debug(1, printAST(*ptr));

                    return 1;
                }
                curEnv = curEnv->parent;
            }

            // Debug(0, "Checking global env.\n");
            // cmp = searchEnvironment(GLOBALCONTEXT->env, ast->identifier);
            // if (cmp !=  NULL && cmp->type == VARIABLE) {
            //     Debug(0, "Replacing '%s' with recursive definition\n", ast->identifier);
            //     free(ast->identifier);
            //     free(ast);

            //     ASTNode *temp = deepCopyAST(cmp->value);
            //     if (temp == NULL) return 0;
            //     if (!replace(&temp, env)) {
            //         freeAST(temp);
            //         return 0;
            //     }
            //     *ptr = temp;
            //     Debug(1, printAST(*ptr));
            //     return 1;
            // }

            return 1;

        case NODE_OPERATOR:
            if (!replace(&ast->left, env)) return 0;
            if (!replace(&ast->right, env)) return 0;
            return 1;
    }

    return 1;
}


static int executeRecur(ASTNode **ptr, Environment *env) {
    ASTNode *ast = *ptr;
    if (ast == NULL) return 0;

    Debug(0, "Executing\n");
    Debug(1, printAST(ast));

    switch (ast->type) {
        case NODE_NUMBER:
            return 1;

        case NODE_VARIABLE:
            if (!replace(ptr, env)) return 0;
            return 1;

        // Adds new function to global environment
        case NODE_ASSIGN_FUNC:
            Info(0, "\nBinding function %s to global environment\n",ast->left->identifier);
            if (!executeRecur(&ast->func->definition, NULL)) {
                freeAST(ast);
                return 0;
            }

            if (!bindComponent(GLOBALCONTEXT->env, FUNCTION, ast->left->identifier, ast->func)) {
                freeAST(ast);
                return 0;
            }
            Debug(1, printEnvironment(GLOBALCONTEXT->env));

            free(ast->left->identifier);
            free(ast->left);
            free(ast);

            *ptr = NULL;

            return 1;

        // Adds new variable to global environment
        case NODE_ASSIGN_VAR:
            Info(0, "\nBinding variable to global environment\n");
            if (!executeRecur(&ast->right, NULL)) {
                freeAST(ast);
                return 0;
            }

            if (!bindComponent(GLOBALCONTEXT->env, VARIABLE, ast->left->identifier, ast->right)) {
                freeAST(ast);
                return 0;
            }
            Debug(1, printEnvironment(GLOBALCONTEXT->env));
            
            free(ast->left->identifier);
            free(ast->left);
            free(ast);


            *ptr = NULL;

            return 1;

        case NODE_OPERATOR:
            Debug(0, "Executing operator children\n");
            int left = executeRecur(&ast->left, env);
            int right = executeRecur(&ast->right, env);

            if (left == 0 || right == 0) return 0;

            // Simplifies constants (doesn't do for division)
            if (ast->left->type == NODE_NUMBER && ast->right->type == NODE_NUMBER) {
                ASTNode *new = dummyASTNode(NODE_NUMBER);
                if (new == NULL) return 0;
                // Add associativity support for * and +

                switch (ast->op) {
                    case OP_ADDITION:
                        new->value = ast->left->value + ast->right->value;
                        break;

                    case OP_SUBTRACTION:
                        new->value = ast->left->value - ast->right->value;
                        break;

                    case OP_MULTIPLICATION:
                        new->value = ast->left->value * ast->right->value;
                        break;

                    case OP_EXPONTENTIATION:
                        new->value = powf(ast->left->value, ast->right->value);
                        break;
                    case OP_DIVISION:
                        // check config for is fractions are preserved
                        break;
                }

                freeAST(ast);
                *ptr = new;


            }
            return 1;

        case NODE_FUNC_CALL:
            FunctionCall *call = ast->call;

            Component *cmp = searchEnvironment(GLOBALCONTEXT->env, call->identifier);
            if (cmp == NULL || cmp->type == VARIABLE) {
                printf("Error getting environment component.\n");
                return 0;
            }

            Function *func = cmp->func;
            if (func == NULL) {
                printf("Function '%s' couldn't be found in the environment.\n", call->identifier);
                return 0;
            }

            if (call->nParams != func->env->entries) {
                printf("Expected %d parameters for '%s', %d parameters were passed.\n", func->env->entries, call->identifier, call->nParams);
                return 0;
            }

            Debug(0, "Updating local environment.\n");
            // Updates values for parameters for this call
            Environment *localEnv = func->env;
            if (env != NULL) localEnv->parent = env;
            int params = call->nParams;
            for (int p = 0; p < params; p ++) {
                freeAST(localEnv->components[p].value);
                if (!replace(&call->parameters[p], env)) {
                    localEnv->parent = NULL;
                    return 0;
                }
                localEnv->components[p].value = deepCopyAST(call->parameters[p]);
            }

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing function call on defined function.\n");
                    Debug(1, printAST(func->definition));
                    Debug(0, "Replacing variables with new definitions\n");

                    ASTNode *exec = deepCopyAST(func->definition);
                    if (exec == NULL) return 0;
                    if (!replace(&exec, localEnv)) return 0;

                    Debug(0, "Executing function body\n");
                    if (exec == NULL || !executeRecur(&exec, localEnv)) {
                        localEnv->parent = NULL;
                        return 0;
                    }

                    localEnv->parent = NULL;
                    freeAST(ast);
                    *ptr = exec;
                    return 1;
            }
            
            return 1;
    }

    return 1;
}

int execute(ASTNode **ast) {
    Info(0, "\nRoot Execution\n");
    if (!executeRecur(ast, GLOBALCONTEXT->env)) return 0;

    Info(0, "Finished Executing\n");
    return 1;
}