#pragma once

#include "core/primitives/types.h"


typedef enum {
    ENV_LIST,
    ENV_HASH
} EnvironmentType; 


typedef struct {
    EnvironmentType type;
    
    union {
        Component *compList;
        Component **hashTable;
    };
    struct Environment *parent;
} Environment;


Environment *createHashEnvironment();


void freeEnvironment(Environment *env);
