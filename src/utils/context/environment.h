#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "utils/types.h"

// Environment related definitions
typedef enum {
    VARIABLE,
    FUNCTION
} ComponentType;

typedef struct {
    ComponentType type;
    char *identifier;

    union {
        Function *function;
        double value;
    };

} Component;

typedef struct {
    int entries;
    int size;
    Component *components;
} Environment;



Environment *createEnvironment();


int bindComponent(Environment *env, ComponentType type, char *identifier, void *component);


Component* searchEnvironment(Environment *env, char *identifier);


void printEnvironment(Environment *env);

#endif