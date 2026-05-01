#ifndef REGISTRY_H
#define REGISTRY_H

#include "core/primitives/types.h"


typedef struct Registry {
	unsigned int operationsSize;
	unsigned int registeredOperations;
	Operation **operations;
} Registry;


Registry *initRegistry();


int registerOperation(Registry *registry, Operation *op);


Operation *searchOperation(Registry *registry, char symbol);


void freeRegistry(Registry *registry);

#endif
