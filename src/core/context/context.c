#include <stdlib.h>
#include <string.h>

#include "core/utils/log.h"
#include "core/utils/expr_utils.h"
#include "core/context.h"
#include "config.h"
#include "environment.h"
#include "registry.h"


Context *GLOBALCONTEXT = NULL;


bool initOutputVariables(Context *ctx) {
    int outputs = ctx->config->OUTPUTS;
	if (outputs <= 0) return true;
	
	if (outputs == 1) {
		Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
		if (temp == NULL) return false;
		temp->value = 0;

		if (!bindComponent(ctx->env, COMP_VARIABLE, ctx->config->OUTPUT_ID, temp)) {
			free(temp);
			return false;
		}
		
		return true;
	}

	for (int i = 0; i < outputs; i ++) {
		int size = strlen(ctx->config->OUTPUT_ID) + 12;

		char *str = malloc(size);
		if (str == NULL) return false;
		snprintf(str, size, "%s_%d", ctx->config->OUTPUT_ID, i);

		Expression *temp = dummyExpression(EXPRESSION_DOUBLE);
		if (temp == NULL) return false;
		temp->value = 0;

		if (!bindComponent(ctx->env, COMP_VARIABLE, str, temp)) {
			free(temp);
			free(str);
			return false;
		}

		free(str);
	}

    return true;
}


bool initContext(char const *cpath) {
    if (GLOBALCONTEXT != NULL) return false;

    Config *config = loadConfig(cpath);
    Environment *env = createEnvironment(); // will change to hash map later
	Registry *registry = initRegistry();

    if (config == NULL || env == NULL || registry == NULL) {
        freeConfig(config);
        freeEnvironment(env);
		freeRegistry(registry);
        return false;
    }
    
	Context *ctx = calloc(1, sizeof(Context));
	if (ctx == NULL) {
        freeConfig(config);
        freeEnvironment(env);
		freeRegistry(registry);
        return false;
	}

	ctx->config = config;
	ctx->env = env;
	ctx->registry = registry;

	GLOBALCONTEXT = ctx;

	printf("setting primitives\n");
	if (!initPrimitives(registry)) {
		freeContext(ctx);
		return false;
	}

	printf("initting output vars\n");
    if (!initOutputVariables(ctx)) {
        freeContext(ctx);
        return false;
    }

	printf("done ctx init\n");
    return true;
}


void freeContext(Context *ctx) {
	freeEnvironment(ctx->env);
	freeConfig(ctx->config);
	freeRegistry(ctx->registry);

    free(ctx);
}


bool reloadConfig(Context *ctx) {
	Config *new = loadConfig(ctx->config->CONFIG_FILE_PATH);

	if (new == NULL) return false;

	freeConfig(ctx->config);
	ctx->config = new;

	return true;
}


bool updateOutputVariables(Context *ctx, Expression *output) {
	if (ctx->config->OUTPUTS <= 0) {
		freeExpression(output);
		return true;
	}

    Debug(0, "Updating output variable(s).\n");
    if (ctx->config->OUTPUTS == 1) {
        Component *cmp = _searchEnvironment(ctx->env, ctx->config->OUTPUT_ID);
        if (cmp == NULL) return false;

        freeExpression(cmp->value);
        cmp->value = output;
        return true;
    } else {
        int size = strlen(ctx->config->OUTPUT_ID) + 12;
        char *str = malloc(size);
        if (str == NULL) return false;
        snprintf(str, size, "%s_%d", ctx->config->OUTPUT_ID, ctx->config->OUTPUTS - 1);

        Component *last = _searchEnvironment(ctx->env, str);
        if (last == NULL) {
            free(str);
            return false;
        }
        free(str);
        freeExpression(last->value);

        for (int i = ctx->config->OUTPUTS - 2; i >= 0; i --) {
            size = strlen(ctx->config->OUTPUT_ID) + 12;
            str = malloc(size);
            if (str == NULL) return false;
            snprintf(str, size, "%s_%d", ctx->config->OUTPUT_ID, i);

            Component *cmp = _searchEnvironment(ctx->env, str);
            if (cmp == NULL) {
                free(str);
                return false;
            }
            free(str);

            last->value = cmp->value;
            last = cmp;
        }

        size = strlen(ctx->config->OUTPUT_ID) + 12;
        str = malloc(size);
        if (str == NULL) return false;
        snprintf(str, size, "%s_0", ctx->config->OUTPUT_ID);

        Component *first = _searchEnvironment(ctx->env, str);
        if (first == NULL) {
            free(str);
            return false;
        }
        free(str);

        first->value = output;

        return true;
    }

    return true;
}
