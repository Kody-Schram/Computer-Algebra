#include <stdlib.h>
#include <stdio.h>

#include "execute.h"
#include "utils/context/context.h"
#include "utils/log.h"

int execute(ASTNode *ast) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    Debug("Executing\n");

    // Updates environment if an assignment is returned
    switch (ast->type) {
        case NODE_ASSIGN_FUNC:
            Info("\nBinding function %s to global environment\n", ast->left->identifier);

            bindComponent(env, FUNCTION, ast->left->identifier, ast->right->func);
            if (config->LOG_LEVEL >= DEBUG) {
                printEnvironment(ast->right->func->env);
            }

            // Free assignment nodes after assignment
            free(ast->left->identifier);
            free(ast->left);

            free(ast->right);
            break;
        case NODE_ASSIGN_VAR:
            Info("\nBinding variable to global environment\n");
            break;
        default:
            
    }

    Debug("Successfully Executed.\n");

    return 1;
}