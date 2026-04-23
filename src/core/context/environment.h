#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdio.h>
#include "core/primitives/types.h"

typedef enum ComponentType ComponentType;
typedef struct Component Component;
typedef enum EnvironmentType EnvironmentType;
typedef struct Environment Environment;


// Environment related definitions
enum ComponentType {
    COMP_OPERATION,
    COMP_VARIABLE,
    COMP_FUNCTION
};

struct Component {
    ComponentType type;
    char *identifier;
    // Pack secondary hashing to have super fast comparisions when tokenizing
    Component *next;

    union {
        Function *func;
        Expression *value;
        Operation *operation;
    };
};

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


int bindComponent(Environment *env, ComponentType type, const char *identifier, const void *data);


Component* searchEnvironment(const Environment *env, const char *identifier);


FILE *printEnvironment(const Environment *env);


void freeEnvironment(Environment *env);


int initOutputVariables(Environment *env);


int updateOutputVariables(Environment *env, Expression *output);

#endif