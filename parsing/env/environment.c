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
    env->symbols = malloc(sizeof(Symbol) * DEFAULT_TABLE_SIZE);

    return env;
}

int bindSymbol(Environment *env, SymbolType type, char *identifier, void *symbol) {
    if (env->entries >= env->size) {
        env->size += DEFAULT_INCREASE_SIZE;
        Symbol* temp = realloc(env->symbols, env->size);
        if (temp == NULL) {
            printf("Error reallocating space for environment.\n");
            return 0;
        }

        env->symbols = temp;
    }

    // Creates new symbol
    Symbol *new = &env->symbols[env->entries];
    new->type = type;
    new->identifier = strdup(identifier);
    if (type == VARAIBLE) new->value = *(double*) symbol;
    else new->function = (Function*) symbol;

    env->entries ++;

    return 1;

}

Symbol* searchEnvironment(Environment *env, char *identifier) {
    for (int i = 0; i < env->entries; i ++) {
        if (strcmp(env->symbols[i].identifier, identifier) == 0) {
            return &env->symbols[i];
        }
    }

    return NULL;
}

