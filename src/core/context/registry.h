#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


typedef struct ObjectRegistry {
	char *identifier;
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


Operation const *searchOperation(Registry const *registry, char symbol);


bool registerOperation(Registry *registry, Operation *op);


bool registerObject(
		Registry *registry, char const *identifier,
		void (*cleanup) (void *data), int32_t (*compare) (void const *ptr)
);


void freeRegistry(Registry *registry);
