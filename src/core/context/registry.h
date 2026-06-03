#pragma once

#include <stdint.h>
#include "core/common.h"


// Mapping lists make lookups have better cache locality and vectorization
typedef struct Registry {
	uint32_t operationsSize;
	uint32_t registeredOperations;
	char *operation_mapping;
	Operation *operations;

	uint32_t objectsSize;
	uint32_t registeredObjects;
	uint64_t *object_mapping;
	Object *objects;
} Registry;


Registry *initRegistry();


void freeRegistry(Registry *registry);


bool initPrimitives(Registry *registry);
