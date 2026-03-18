#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "environment.h"

const int DEFAULT_TABLE_SIZE = 10;
const int DEFAULT_INCREASE_SIZE = 5;

Environment *createEnvironment() {
    Environment *env = malloc(sizeof(Environment));
    env->entries = 0;
    env->size = DEFAULT_TABLE_SIZE;
    env->components = malloc(sizeof(Component) * DEFAULT_TABLE_SIZE);

    return env;
}

int bindComponent(Environment *env, ComponentType type, char *identifier, void *component) {
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
    if (type == VARIABLE) new->value = *(double*) component;
    else new->function = (Function*) component;

    env->entries ++;

    return 1;

}

Component* searchEnvironment(Environment *env, char *identifier) {
    for (int i = 0; i < env->entries; i ++) {
        if (strcmp(env->components[i].identifier, identifier) == 0) {
            return &env->components[i];
        }
    }

    return NULL;
}


void printEnvironment(Environment *env) {
    for (int i = 0; i < env->entries; i ++) {
        char *type;
        switch(env->components[i].type) {
            case FUNCTION:
                type = "function";
                break;
            default:
                type = "variable";
        }
        
        printf("%s (%s)", env->components[i].identifier, type);
    }
}
