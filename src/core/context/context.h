#ifndef CONTEXT_H
#define CONTEXT_H

#include "core/primitives/types.h"
#include "environment.h"
#include "config.h"
#include "registry.h"

typedef struct {
    Environment *env;
    Config *config;
	Registry *registry;
} Context;


extern Context *GLOBALCONTEXT;


Context *createContext(Config *config, Environment *env, Registry *registry);


int initContext(char *cpath);


void freeContext(Context *context);


#endif
