#ifndef CONTEXT_H
#define CONTEXT_H

#include "core/primitives/types.h"
#include "environment.h"
#include "config.h"

typedef struct {
    Environment *env;
    Config *config;
} Context;


extern Context *GLOBALCONTEXT;


Context *createContext(Config *config, Environment *env);


int initContext(char *cpath);


void freeContext(Context *context);


#endif