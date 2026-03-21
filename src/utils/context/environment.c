#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "context.h"

const int DEFAULT_TABLE_SIZE = 10;
const int DEFAULT_INCREASE_SIZE = 5;

Environment *createEnvironment() {
    Environment *env = malloc(sizeof(Environment));
    env->entries = 0;
    env->size = DEFAULT_TABLE_SIZE;
    env->components = malloc(sizeof(Component) * DEFAULT_TABLE_SIZE);

    return env;
}


int bindComponent(Environment *env, ComponentType type, char *identifier, void *data) {
    if (env->entries >= env->size) {
        env->size += DEFAULT_INCREASE_SIZE;
        Component* temp = realloc(env->components, sizeof(Component) * env->size);
        if (temp == NULL) {
            printf("Error reallocating space for environment.\n");
            return 0;
        }

        env->components = temp;
    }

    // Creates new component
    Component *new = &env->components[env->entries];
    new->type = type;
    new->identifier = strdup(identifier);

    if (type == FUNCTION) new->func = (Function *) data;
    else new->value = (ASTNode *) data;

    env->entries ++;
    return 1;

}


Component* searchEnvironment(Environment *env, char *identifier) {
    for (int i = 0; i < env->entries; i ++) {
        if (!strcmp(env->components[i].identifier, identifier)) {
            return &env->components[i];
        }
    }

    return NULL;
}


void printEnvironment(Environment *env) {
    Config *config = GLOBALCONTEXT->config;

    for (int i = 0; i < env->entries; i ++) {
        switch(env->components[i].type) {
            case FUNCTION:
                Function *func = env->components[i].func;

                if (env->components[i].func->env->entries > 0) {
                    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "%s(", env->components[i].identifier);
                    for (int j = 0; j < func->env->entries - 1; j ++) {
                        if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "%s,", func->env->components[j].identifier);
                    }
                    if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "%s)\n", func->env->components[func->env->entries-1].identifier);
                    break;
                }
            default:
                if (config->LOG_LEVEL >= DEBUG) fprintf(config->LOG_STREAM, "%s = %f (variable)\n", env->components[i].identifier, env->components[i].value);
        }
        
    }
}
