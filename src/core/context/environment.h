#pragma once

#include <stdio.h>
#include "core/primitives/types.h"

typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef enum EnvironmentType EnvironmentType;
typedef struct Environment Environment;


// Environment related definitions

enum EnvironmentType {
    ENV_LIST,
    ENV_HASH
}; 

struct Environment {
    EnvironmentType type;
    
    union {
        Component *compList;
        Component **hashTable;
    };
    Environment *parent;
};



Environment *createEnvironment(EnvironmentType type);


bool bindComponent(Environment *env, ComponentType type, char const *identifier, void const *data);


Component *searchEnvironment(Environment const *env, char const *identifier);


FILE *printEnvironment(Environment const *env);


void freeEnvironment(Environment *env);


int initOutputVariables(Environment *env);


int updateOutputVariables(Environment *env, Expression *output);
