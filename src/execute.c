#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "execute.h"
#include "utils/types.h"
#include "utils/context/context.h"
#include "utils/log.h"

static const int DEFAULT_ENV_STACK = 3;


static long long gcd(long long a, long long b)
{
    long long temp;
    while (b != 0)
    {
        temp = a % b;

        a = b;
        b = temp;
    }
    return a;
}


static long long powi(long long a, long long e) {
    long long r = 1;

    while (e > 0) {
        if (e % 2 == 1) r *= a;
        a *= a;
        e /= 2;
    }

    return r;
}


static int executeRecur(ASTNode **ptr, Environment *env) {
    ASTNode *ast = *ptr;
    if (ast == NULL) return 0;

    switch (ast->type) {
        case NODE_INTEGER:
        case NODE_DOUBLE:
            return 1;

        case NODE_VARIABLE: {
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

                    *ptr = deepCopyAST(cmp->value);
                    if (!executeRecur(ptr, env)) return 0;
                    Debug(1, printAST(*ptr));

                    return 1;
                }
                curEnv = curEnv->parent;
            }

            return 1;
        }

        // Adds new function to global environment
        case NODE_ASSIGN_FUNC:
            Info(0, "\nBinding function %s to global environment\n",ast->left->identifier);
            if (!executeRecur(&ast->func->definition, NULL)) {
                return 0;
            }

            if (!bindComponent(env, FUNCTION, ast->left->identifier, ast->func)) {
                return 0;
            }
            Debug(1, printEnvironment(env));

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

            if (!bindComponent(env, VARIABLE, ast->left->identifier, ast->right)) {
                freeAST(ast);
                return 0;
            }
            Debug(1, printEnvironment(env));
            
            free(ast->left->identifier);
            free(ast->left);
            free(ast);


            *ptr = NULL;

            return 1;

        case NODE_OPERATOR:
            Debug(0, "Executing operator children\n");
            if (!executeRecur(&ast->left, env) || !executeRecur(&ast->right, env)) return 0;

            // Evaluates if both children aren't a variable
            ASTNode *left = ast->left;
            ASTNode *right = ast->right;
            if (left->type != NODE_VARIABLE && right->type != NODE_VARIABLE) {
                switch (ast->op) {
                    case OP_ADDITION: {
                        ASTNode *new = NULL;
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            new->integer = left->integer + right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            double l = (left->type == NODE_INTEGER) ? (double) left->integer : left->value;
                            double r = (right->type == NODE_INTEGER) ? (double) right->integer : right->value;

                            new->value = r + l;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_SUBTRACTION: {
                        ASTNode *new = NULL;
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            new->integer = left->integer - right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            long double l = (left->type == NODE_INTEGER) ? (long double) left->integer : left->value;
                            long double r = (right->type == NODE_INTEGER) ? (long double) right->integer : right->value;

                            new->value = r - l;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_MULTIPLICATION: {
                        ASTNode *new = NULL;
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            new->integer = left->integer * right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            double l = (left->type == NODE_INTEGER) ? (double) left->integer : left->value;
                            double r = (right->type == NODE_INTEGER) ? (double) right->integer : right->value;

                            new->value = r * l;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_EXPONTENTIATION: {
                        ASTNode *new = NULL;
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            new->integer = powi(left->integer, right->integer);
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            double l = (left->type == NODE_INTEGER) ? (double) left->integer : left->value;
                            double r = (right->type == NODE_INTEGER) ? (double) right->integer : right->value;

                            new->value = powf(l, r);
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_DIVISION: {
                        if (!GLOBALCONTEXT->config->PRESERVE_FRACS) {
                            ASTNode *new = dummyASTNode(NODE_DOUBLE);
                            new->value = ast->left->value / ast->right->value;

                            freeAST(ast);
                            *ptr = new;
                        } else {
                            if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {

                                int g = gcd(left->integer, right->integer);
                                if (g != 1) {
                                    left->integer = left->integer / g;
                                    right->integer = right->integer / g;
                                }
                            }

                            return 1;
                        }
                        break;
                    }
                }

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
                // freeAST(ast);
                // *ptr = NULL;
                return 0;
            }

            // Goes up call stack, if global environment is found, then this is evaluating rather than like binding a new variable/function
            int evaluating = 0;
            Environment *tempEnv = env;
            while (tempEnv != NULL) {
                if (tempEnv == GLOBALCONTEXT->env) { 
                    evaluating = 1;
                    Debug(0, "Global env found, evaluating.\n");
                    break;
                }
                tempEnv = tempEnv->parent;
            }

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return 1;

            Debug(0, "Creating temp local env.\n");

            Environment *localEnv = createEnvironment();
            if (localEnv == NULL) return 0;
            if (env != NULL) localEnv->parent = env;

            int params = call->nParams;
            for (int p = 0; p < params; p ++) {
                // Updates local environment variable definitions with copies of passed in parameters
                if (!executeRecur(&call->parameters[p], localEnv) ) {
                    freeEnvironment(localEnv);
                    return 0;
                }

                if (!bindComponent(localEnv, VARIABLE, func->env->components[p].identifier, call->parameters[p])) {
                    freeEnvironment(localEnv);
                    return 0;
                }

                // Updates parameters with outer variables
                if (!executeRecur(&localEnv->components[p].value, env)) {
                    freeEnvironment(localEnv);
                    return 0;
                }
            }

            switch (func->type) {
                case DEFINED:
                    Debug(0, "Executing function call on defined function.\n");
                    Debug(1, printAST(func->definition));
                    Debug(0, "Replacing variables with new definitions\n");

                    ASTNode *exec = deepCopyAST(func->definition);
                    if (exec == NULL) return 0;
                    if (!executeRecur(&exec, localEnv)) return 0;

                    localEnv->parent = NULL;
                    call->nParams = 0;
                    freeAST(ast);
                    freeEnvironment(localEnv);
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