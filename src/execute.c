#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "execute.h"
#include "utils/types.h"
#include "utils/context/context.h"
#include "utils/log.h"

static const int DEFAULT_ENV_STACK = 3;

static ASTNode *copyAndReplace(ASTNode *ast, Environment *env);
static int executeRecur(ASTNode **ptr, ASTNode *parent, Environment *env);


static ASTNode *copyAndReplace(ASTNode *ast, Environment *env) {
    if (ast == NULL) return NULL;

    ASTNode *cur = dummyASTNode(ast->type);
    if (cur == NULL) {
        printf("Error creating dummy copy ASTNode.\n");
        return NULL;
    }

    switch (cur->type) {
        case NODE_NUMBER:
            cur->value = ast->value;
            break;
        case NODE_VARIABLE:
            Component *cmp = NULL;
            Environment *curEnv = env;
            while (cmp == NULL && curEnv != NULL) {
                if (cur->type != NODE_VARIABLE) break;
                cmp = searchEnvironment(curEnv, ast->identifier);

                if (cmp !=  NULL && cmp->type == VARIABLE) {
                    Debug(0, "Replacing '%s' with recursive definition\n", cmp->identifier);
                    free(cur);
                    cur = copyAndReplace(cmp->value, env);
                    Debug(1, printAST(cur));
                    return cur;
                }
                curEnv = curEnv->parent;
            }

            Debug(0, "Checking global env.\n");
            cmp = searchEnvironment(GLOBALCONTEXT->env, ast->identifier);
            if (cmp !=  NULL && cmp->type == VARIABLE) {
                Debug(0, "Replacing '%s' with recursive definition\n", cmp->identifier);
                free(cur);
                cur = copyAndReplace(cmp->value, env);
                Debug(1, printAST(cur));
                return cur;
            }

            cur->identifier = strdup(ast->identifier);
            break;
        case NODE_OPERATOR:
            cur->op = ast->op;
            break;
        case NODE_FUNC_CALL:
            FunctionCall *call = ast->call;
            Debug(0, "Copying call '%s'\n", call->identifier);
            cur->call = malloc(sizeof(FunctionCall));
            cur->call->identifier = strdup(call->identifier);
            cur->call->nParams = call->nParams;

            cmp = searchEnvironment(GLOBALCONTEXT->env, cur->call->identifier);
            if (cmp == NULL || cmp->type != FUNCTION) {
                printf("Error getting environment component.\n");
                return NULL;
            }

            // Makes deep copy of call, this way it doesnt override the call in the function definition in the environment
            cur->call->parameters = malloc(sizeof(ASTNode *) * cur->call->nParams);
            for (int p = 0; p < call->nParams; p ++) {
                cur->call->parameters[p] = copyAndReplace(call->parameters[p], NULL);
                if (cur->call->parameters[p] == NULL) return NULL;
            }

            Function *func = cmp->func;
            if (func == NULL) {
                printf("Function '%s' couldn't be found in the environment.\n", cur->call->identifier);
                return NULL;
            }

            for (int p = 0; p < cur->call->nParams; p ++) {
                ASTNode *param = copyAndReplace(cur->call->parameters[p], env);
                if (param == NULL) return NULL;
                Debug(0, "Execting nested call\n");
                if (!executeRecur(&param, NULL, env)) return NULL;

                freeAST(cur->call->parameters[p]);
                cur->call->parameters[p] = param;
            }

            if (!executeRecur(&cur, NULL, env)) return NULL;
            break;
        default:
            Debug(0, "This shouldnt happen... (%d)\n", ast->type);
    }

    cur->left = copyAndReplace(ast->left, env);
    cur->right = copyAndReplace(ast->right, env);

    return cur;
}


static int executeRecur(ASTNode **ptr, ASTNode *parent, Environment *env) {
    ASTNode *ast = *ptr;
    if (ast == NULL) return 1;

    int left = 1;
    int right = 1;

    if (ast->type != NODE_ASSIGN_FUNC && ast->type != NODE_ASSIGN_VAR && ast->type != NODE_FUNC_CALL) {
        Debug(0, "Executing children\n");
        // Executes bottom up
        left = executeRecur(&ast->left, ast, env);
        right = executeRecur(&ast->right, ast, env);
    }

    if (left == 0 || right == 0) return 0;

    Debug(0, "Executing\n");
    Debug(1, printAST(ast));

    Config *config = GLOBALCONTEXT->config;

    // Updates environment if an assignment is returned
    switch (ast->type) {
        case NODE_VARIABLE:
        case NODE_NUMBER:
            return 1;

        case NODE_ASSIGN_FUNC:
            Info(0, "\nBinding function %s to global environment\n", (char *) ast->left);
            executeRecur(&ast->func->definition, NULL, NULL);

            bindComponent(GLOBALCONTEXT->env, FUNCTION, (char *) ast->left, ast->func);
            Debug(1, printEnvironment(ast->func->env));

            free(ast->left);
            free(ast);
            *ptr = NULL;
            return 1;

        case NODE_ASSIGN_VAR:
            Info(0, "\nBinding variable to global environment\n");
            freeAST(ast);
            return 1;
            break;

        case NODE_OPERATOR:
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
                if (parent == NULL) *ptr = new;
                else if (parent->left == ast) {
                    parent->left = new;
                } else {
                    parent->right = new;
                }

            }
            break;

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
                localEnv->components[p].value = copyAndReplace(call->parameters[p], env);
            }

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing function call on defined function.\n");
                    Debug(1, printAST(func->definition));
                    Debug(0, "Replacing variables with new definitions\n");
                    ASTNode *exec = copyAndReplace(func->definition, localEnv);

                    Debug(0, "Executing function body\n");
                    if (exec == NULL || !executeRecur(&exec, NULL, localEnv)) {
                        localEnv->parent = NULL;
                        return 0;
                    }

                    localEnv->parent = NULL;
                    *ptr = exec;
                    break;
            }
            
            break;
    }

    return 1;
}

int execute(ASTNode *ast) {
    Info(0, "\nRoot Execution\n");
    // printf("exec\n");

    if (executeRecur(&ast, NULL, NULL)) {
        Info(0, "Finished Executing\n");
        if (ast != NULL) printStream(printAST(ast));
        
        Debug(0, "Freeing result\n");
        freeAST(ast);
        return 1;
    }

    return 0;
}