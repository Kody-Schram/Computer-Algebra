#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "execute.h"
#include "utils/context/context.h"
#include "parsing/parserUtils.h"
#include "utils/log.h"

int execute(ASTNode **ptr, ASTNode *parent) {
    ASTNode *ast = *ptr;
    if (ast == NULL) return 1;

    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    Debug("Executing\n");
    if (config->LOG_LEVEL >= DEBUG) printAST(ast);

    // Updates environment if an assignment is returned
    switch (ast->type) {
        case NODE_ASSIGN_FUNC:
            Info("\nBinding function %s to global environment\n", ast->left->identifier);

            bindComponent(env, FUNCTION, ast->left->identifier, ast->right->func);
            if (config->LOG_LEVEL >= DEBUG) {
                printEnvironment(ast->right->func->env);
            }

            freeAST(ast);
            break;
        case NODE_ASSIGN_VAR:
            Info("\nBinding variable to global environment\n");
            freeAST(ast);
            break;
        case NODE_OPERATOR:
            Debug("Executing operator\n");
            // Simplifies constants (doesn't do for division)
            if (ast->left->type == NODE_NUMBER && ast->right->type == NODE_NUMBER) {
                ASTNode *new = dummyASTNode(NODE_NUMBER);
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
                if (parent == NULL) **ptr = *new;
                else *parent = *new;
            } else {
                if (!execute(&ast->left, ast)) return 0;
                if (!execute(&ast->right, ast)) return 0;
            }
        default:
            if (!execute(&ast->left, ast)) return 0;
            if (!execute(&ast->right, ast)) return 0;
            
    }

    Debug("Successfully Executed.\n");
    printAST(ast);

    return 1;
}