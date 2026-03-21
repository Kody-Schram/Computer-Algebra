#include <stdlib.h>
#include <stdio.h>

#include "execute.h"
#include "utils/context/context.h"
#include "utils/log.h"

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

        bindComponent(env, type, ast->left->identifier, ast->right->func);
        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "Binded to global env.\n");
        if (config->LOG_LEVEL >= DEBUG) {
            //Function *func = ast->right->func;
            // fprintf(config->LOG_STREAM, "Handling assingment of '%s'.\nPrinting function\n", ast->left->identifier);
            // fprintf(config->LOG_STREAM, "Function: '%s(", ast->left->identifier);

            // for (int j = 0; j < func->env->entries - 1; j ++) {
            //     fprintf(config->LOG_STREAM, "%s,", func->env->components[j].identifier);
            // }
            // fprintf(config->LOG_STREAM, "%s)\n", func->env->components[func->env->entries-1].identifier);
            printEnvironment(ast->right->func->env);
        }

        // Free assignment nodes after assignment
        free(ast->left->identifier);
        free(ast->left);

        free(ast->right);
    }

    Debug("Successfully Executed.\n");

    return 1;
}