#include <stdlib.h>
#include <stdio.h>

#include "context.h"
#include "core/utils/context/environment.h"

Context *GLOBALCONTEXT = nullptr;

Context *createContext(Config *config, Environment *env) {
    Context *context = calloc(1, sizeof(Context));
    if (context == nullptr) {
        perror("Error in context");
        return nullptr;
    }

    context->config = config;
    context->env = env;

    return context;
}

int initContext(char *cpath) {
    if (GLOBALCONTEXT != nullptr) return 0;

    printf("loading config\n");
    Config *config = loadConfig(cpath);
    printf("loading env\n");
    Environment *env = createEnvironment(ENV_LIST); // will change to hash map later

    if (config == nullptr || env == nullptr) {
        freeConfig(config);
        freeEnvironment(env);
        return 0;
    }
    
    GLOBALCONTEXT = createContext(config, env);
    if (GLOBALCONTEXT == nullptr) {
        freeConfig(config);
        freeEnvironment(env);
        return 0;
    }

    if (!initOutputVariables(env)) {
        freeContext(GLOBALCONTEXT);
        return 0;
    }

    return 1;
}

void freeContext(Context *context) {
    if (context != nullptr) {
        freeEnvironment(context->env);
        freeConfig(context->config);
    }
    free(context);
}