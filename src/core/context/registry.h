#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


typedef struct Registry {
	uint32_t operationsSize;
	uint32_t registeredOperations;
	Operation *operations;

	uint32_t objectsSize;
	uint32_t registeredObjects;
	Object *objects;
} Registry;


Registry *initRegistry();


Operation const *searchOperation(Registry const *registry, char symbol);


bool registerOperation(Registry *registry, Operation op);


bool registerObject(Registry *registry, Object obj);


Object const *searchObject(Registry const *registry, uint64_t obj_id);


void freeRegistry(Registry *registry);


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t operands);


bool createObject(
		Object *out,
		char const *identifier, char const *originModule,
		void (*cleanup)(void *data), int32_t (*compare)(void const *a, void const *b)
);

bool initPrimitives(Registry *registry);
