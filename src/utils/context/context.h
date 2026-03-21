#ifndef CONTEXT_H
#define CONTEXT_H

#include "utils/types.h"
#include "environment.h"
#include "config.h"

typedef struct {
    Environment *env;
    Config *config;
} Context;


extern Context *GLOBALCONTEXT;


Context *createContext(Config *config, Environment *env);


int initContext();


#endif