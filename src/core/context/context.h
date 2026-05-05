#pragma once

#include "core/primitives/types.h"
#include "environment.h"
#include "config.h"
#include "registry.h"

struct Context {
    Environment *env;
    Config *config;
	Registry *registry;
};


extern Context *GLOBALCONTEXT;


Context *createContext(Config *config, Environment *env, Registry *registry);


int initContext(char *cpath);


void freeContext(Context *context);
