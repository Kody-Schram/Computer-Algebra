#pragma once

#include <stdint.h>
#include "core/common.h"


// Mapping lists make lookups have better cache locality and vectorization
typedef struct Registry {
	uint32_t operationsSize;
	uint32_t registeredOperations;
	char *operation_map;
	Operation *operations;

	uint32_t objectsSize;
	uint32_t registeredObjects;
	uint64_t *object_map;
	Object *objects;

	bool (*numberParser)(char const *input, ObjectValue *value, uint32_t *flags);
} Registry;


Registry *initRegistry();


void freeRegistry(Registry *registry);


bool initPrimitives(Registry *registry);
