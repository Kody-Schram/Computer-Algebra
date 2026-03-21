#include <stdlib.h>
#include <stdio.h>

#include "execute.h"
#include "utils/context/context.h"

int execute(ASTNode *ast) {
    Config *config = GLOBALCONTEXT->config;
    Environment *env = GLOBALCONTEXT->env;

    // Updates environment if an assignment is returned
    if (ast->type == NODE_ASSIGN_FUNC || ast->type == NODE_ASSIGN_VAR) {
        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Updating environment\n");
        
        ComponentType type;
        switch(ast->type) {
            case NODE_ASSIGN_FUNC:
                type = FUNCTION;
                break;
            default:
                type = VARIABLE;
        }

        bindComponent(env, type, ast->left->identifier, ast->right);

        // Free assignment nodes after assignment
        free(ast->left->identifier);
        free(ast->left);

        free(ast->right);
    }

    return 1;
}