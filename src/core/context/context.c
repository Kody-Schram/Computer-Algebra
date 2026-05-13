#include <stdlib.h>
#include <stdio.h>

#include "context.h"
#include "core/context/environment.h"
#include "core/context/registry.h"


Context *GLOBALCONTEXT = NULL;


Context *createContext(Config *config, Environment *env, Registry *registry) {
    Context *context = calloc(1, sizeof(Context));
    if (context == NULL) {
        perror("Error in context");
        return NULL;
    }

    context->config = config;
    context->env = env;
	context->registry = registry;

    return context;
}


bool initContext(char *cpath) {
    if (GLOBALCONTEXT != NULL) return false;

    Config *config = loadConfig(cpath);
    Environment *env = createEnvironment(ENV_LIST); // will change to hash map later
	Registry *registry = initRegistry();

    if (config == NULL || env == NULL || registry == NULL) {
        freeConfig(config);
        freeEnvironment(env);
		freeRegistry(registry);
        return false;
    }
    
    GLOBALCONTEXT = createContext(config, env, registry);
    if (GLOBALCONTEXT == NULL) {
        freeConfig(config);
        freeEnvironment(env);
		freeRegistry(registry);
        return false;
    }

	if (!initPrimitives(registry)) {
		freeContext(GLOBALCONTEXT);
		return false;
	}

    if (!initOutputVariables(env)) {
        freeContext(GLOBALCONTEXT);
        return false;
    }

    return true;
}

void freeContext(Context *context) {
	freeEnvironment(context->env);
	freeConfig(context->config);
	freeRegistry(context->registry);

    free(context);
}
