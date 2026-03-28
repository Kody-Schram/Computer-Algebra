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
    env->parent = NULL;

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
    if (identifier == NULL) return NULL;
    for (int i = 0; i < env->entries; i ++) {
        if (!strcmp(env->components[i].identifier, identifier)) {
            return &env->components[i];
        }
    }

    return NULL;
}


FILE *printEnvironment(Environment *env) {
    FILE *stream = tmpfile();
    if (stream == NULL || env == NULL) return NULL;

    if (env->entries == 0) {
        fprintf(stream, "Environment Empty.\n");
        return stream;
    }
    
    for (int i = 0; i < env->entries; i ++) {
        switch(env->components[i].type) {
            case FUNCTION:
                Function *func = env->components[i].func;
                if (env->components[i].func->env->entries > 0) {
                    fprintf(stream, "%s(", env->components[i].identifier);
                    for (int j = 0; j < func->env->entries - 1; j ++) {
                        fprintf(stream, "%s,", func->env->components[j].identifier);
                    }
                    fprintf(stream, "%s) = ", func->env->components[func->env->entries-1].identifier);
                }
                
                char *str = astToString(func->definition);
                if (str == NULL) return stream;
                fprintf(stream, "%s\n", str);
                free(str);
                
                break;
                
            case VARIABLE:
                fprintf(stream, "%s = ", env->components[i].identifier);
                
                str = astToString(env->components[i].value);
                if (str == NULL) return stream;
                fprintf(stream, "%s\n", str);
                free(str);

                break;
        }
        
    }

    return stream;
}


void freeEnvironment(Environment *env) {
    if (env != NULL && env->components != NULL) {
        for (int c = 0; c < env->entries; c ++) {
            Component *cmp = &env->components[c];
            free(cmp->identifier);

            if (env->components[c].type == FUNCTION) {
                freeEnvironment(cmp->func->env);
                
                switch (cmp->func->type) {
                    case DEFINED:
                        freeAST(cmp->func->definition);
                        break;
                    case BUILTIN:
                        free(cmp->func->builtin);
                        break;
                    case TRANSFORM:
                        free(cmp->func->transform);
                        break;
                }
                free(cmp->func);
            } else freeAST(cmp->value);
        }
        free(env->components);
    }

    free(env);
}


int initOutputVariables(Environment *env) {
    int outputs = GLOBALCONTEXT->config->OUTPUTS;
    if (outputs > 0) {
        if (outputs == 1) {
            ASTNode *temp = dummyASTNode(NODE_DOUBLE);
            if (temp == NULL) return 0;
            temp->value = 0;

            if (!bindComponent(env, VARIABLE, GLOBALCONTEXT->config->OUTPUT_ID, temp)) {
                free(temp);
                return 0;
            }
            return 1;
        }

        for (int i = 0; i < outputs; i ++) {
            int size = strlen(GLOBALCONTEXT->config->OUTPUT_ID) + 12;

            char *str = malloc(size);
            if (str == NULL) return 0;
            snprintf(str, size, "%s_%d", GLOBALCONTEXT->config->OUTPUT_ID, i);

            ASTNode *temp = dummyASTNode(NODE_DOUBLE);
            if (temp == NULL) return 0;
            temp->value = 0;

            if (!bindComponent(env, VARIABLE, str, temp)) {
                free(temp);
                free(str);
                return 0;
            }

            free(str);
        }
    }

    return 1;
}