#include <stdlib.h>
#include <stdio.h>

#include "context.h"

Context *GLOBALCONTEXT = NULL;

Context *createContext(Config *config, Environment *env) {
    Context *context = malloc(sizeof(Context));
    if (context == NULL) {
        printf("Error allocating for context.\n");
        return NULL;
    }

    context->config = config;
    context->env = env;

    return context;
}

int initContext() {
    if (GLOBALCONTEXT != NULL) return 0;

    Config *config = loadConfig();
    Environment *env = createEnvironment();

    if (config == NULL || env == NULL) return 0;
    GLOBALCONTEXT = createContext(config, env);
    if (GLOBALCONTEXT == NULL) return 0;

    return 1;
}