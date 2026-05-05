#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


typedef struct ObjectRegistry {
	Object *obj;
	void (*cleanup) (void *data);
	int32_t (*compare) (void const *ptr);
} ObjectRegistry;


typedef struct Registry {
	uint32_t operationsSize;
	uint32_t registeredOperations;
	Operation **operations;

	uint32_t objectsSize;
	uint32_t registeredObjects;
	ObjectRegistry *objects;
} Registry;



Registry *initRegistry();


bool registerOperation(Registry *registry, Operation *op);


Operation const *searchOperation(Registry const *registry, char symbol);


bool registerObject(
		Registry *registry, Object *obj,
		void (*cleanup) (void *data), int32_t (*compare) (void const *ptr)
);


Object const *searchObject(Registry const *registry, char const *id);


void freeRegistry(Registry *registry);
