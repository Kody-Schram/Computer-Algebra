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

    Debug(0, "Recursively executing\n");
    Debug(1, printAST(ast));

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
                    Debug(1, printEnvironment(curEnv));
                    free(ast->identifier);
                    free(ast);

                    *ptr = deepCopyAST(cmp->value);
                    if (!executeRecur(ptr, env)) return 0;

                    return 1;
                }
                curEnv = curEnv->parent;
            }

            return 1;
        }

        // Adds new function to global environment
        case NODE_ASSIGN_FUNC:
            Debug(0, "\nBinding function %s to global environment\n",ast->left->identifier);
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
            Debug(0, "\nBinding variable to global environment\n");
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
            int lAst = executeRecur(&(ast->left), env);
            int rAst = executeRecur(&(ast->right), env);
            if (!lAst || !rAst) return 0;

            Debug(0, "Running main operator now\n");
            Debug(1, printAST(ast));
            // Evaluates if both children aren't a variable
            ASTNode *left = ast->left;
            ASTNode *right = ast->right;
            if ((left->type == NODE_INTEGER || left->type == NODE_DOUBLE) && (right->type == NODE_INTEGER || right->type == NODE_DOUBLE)) {
                Debug(0, "Evaluating operation\n");

                ASTNode *new = NULL;
                double l = (left->type == NODE_INTEGER) ? (double) left->integer : left->value;
                double r = (right->type == NODE_INTEGER) ? (double) right->integer: right->value;
                
                switch (ast->op) {
                    case OP_ADDITION: {
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            if (new == NULL) return 0;
                            new->integer = left->integer + right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            if (new == NULL) return 0;

                            new->value = r + l;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_SUBTRACTION: {
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            if (new == NULL) return 0;
                            new->integer = left->integer - right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            if (new == NULL) return 0;

                            new->value = l - r;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_MULTIPLICATION: {
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            new = dummyASTNode(NODE_INTEGER);
                            if (new == NULL) return 0;
                            new->integer = left->integer * right->integer;
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            if (new == NULL) return 0;
            
                            new->value = r * l;
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_EXPONTENTIATION: {
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER && right->integer >= 0) {
                            if (left->integer == 0 && right->integer == 0) {
                                new = dummyASTNode(NODE_INTEGER);
                                if (new == NULL) return 0;
                                new->integer = 1;
                            } else {
                                new = dummyASTNode(NODE_INTEGER);
                                if (new == NULL) return 0;
                                new->integer = powi(left->integer, right->integer);
                            }
                        } else {
                            new = dummyASTNode(NODE_DOUBLE);
                            if (new == NULL) return 0;
                    
                            if (left->value == 0 && right->value == 0) new->value = 1;
                            else new->value = powl(l, r);
                        }

                        freeAST(ast);
                        *ptr = new;
                        return 1;
                    }

                    case OP_DIVISION: {
                        if (left->type == NODE_INTEGER && right->type == NODE_INTEGER) {
                            if (right->integer == 0) {
                                printf("Division by 0.\n");
                                return 0;
                            }

                            // Evaluate fraction
                            if (!GLOBALCONTEXT->config->PRESERVE_FRACS) {
                                new = dummyASTNode(NODE_DOUBLE);
                                if (new == NULL) return 0;
                                new->value = ((double) ast->left->value) / ((double) ast->right->value);

                                freeAST(ast);
                                *ptr = new;
                                return 1;
                            }

                            // Simplify fraction
                            int g = gcd(left->integer, right->integer);
                            if (g != 1) {
                                left->integer = left->integer / g;
                                right->integer = right->integer / g;
                            }
                            return 1;
                        } else {
                            if (right->value == 0) {
                                printf("Division by 0.\n");
                                return 0;
                            }

                            new = dummyASTNode(NODE_DOUBLE);
                            if (new == NULL) return 0;

                            new->value = l / r;
                            freeAST(ast);
                            *ptr = new;

                            return 1;
                        }
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
                return 0;
            }

            // Goes up call stack, if global environment is found, then this is evaluating rather than like binding a new variable/function
            int evaluating = 0;
            Environment *tempEnv = env;
            while (tempEnv != NULL) {
                if (tempEnv == GLOBALCONTEXT->env) { 
                    evaluating = 1;
                    break;
                }
                tempEnv = tempEnv->parent;
            }

            if (!evaluating && GLOBALCONTEXT->config->LAZY_CALLS) return 1;

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
                    Debug(0, "Executing defined function\n");
                    Debug(1, printEnvironment(localEnv));
                    ASTNode *exec = deepCopyAST(func->definition);
                    if (exec == NULL) return 0;
                    if (!executeRecur(&exec, localEnv)) {
                        freeEnvironment(localEnv);
                        return 0;
                    }

                    localEnv->parent = NULL;
                    call->nParams = 0;
                    freeAST(ast);
                    freeEnvironment(localEnv);

                    Debug(1, printAST(exec));
                    *ptr = exec;
                    return 1;

                default:
                    printf("Undefined behavior as of now.\n");
            }
            
            return 1;
    }

    return 1;
}

int execute(ASTNode **ast) {
    Info(0, "\nExecuting\n");
    Info(1, printAST(*ast));
    if (!executeRecur(ast, GLOBALCONTEXT->env)) return 0;

    return 1;
}