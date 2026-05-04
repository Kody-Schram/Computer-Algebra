#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


typedef struct Registry {
	uint32_t operationsSize;
	uint32_t registeredOperations;
	Operation **operations;
} Registry;


Registry *initRegistry();


bool registerOperation(Registry *registry, Operation *op);


Operation *searchOperation(const Registry *registry, char symbol);


void freeRegistry(Registry *registry);
