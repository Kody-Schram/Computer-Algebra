#pragma once

#include "core/common.h"


typedef enum {
    ENV_LIST,
    ENV_HASH
} EnvironmentType; 


typedef struct Environment {
    EnvironmentType type;
    
    union {
        Component *compList;
        Component **hashTable;
    };
    struct Environment *parent;
} Environment;


Environment *createHashEnvironment();


Component *_searchEnvironment(Environment const *env, char const *identifier);


void deepFreeEnvironment(Environment *env);
