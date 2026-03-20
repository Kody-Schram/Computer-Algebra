#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "utils/types.h"


Environment *createEnvironment();


int bindComponent(Environment *env, ComponentType type, char *identifier, void *component);


Component* searchEnvironment(Environment *env, char *identifier);


void printEnvironment(Environment *env);

#endif