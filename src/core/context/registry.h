#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


typedef struct ObjectRegistry {
	char *identifier;
	char *originModule;
	void (*cleanup)(void *data);
	int32_t (*compare)(void const *a, void const *b);
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
		Registry *registry, char const *identifier, char const *originModule,
		void (*cleanup) (void *data), int32_t (*compare) (void const *ptr)
);


uint32_t getObjectId(char const *identifier);


void freeRegistry(Registry *registry);
