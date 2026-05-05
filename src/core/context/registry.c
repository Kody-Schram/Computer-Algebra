#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "registry.h"
#include "core/primitives/types.h"

// +, -, *, /, ^
#define DEFAULT_OPERATIONS 5


Registry *initRegistry() {
	Registry *registry = NULL;
	Operation **operations = NULL;

	registry = malloc(sizeof(Registry));
	if (registry == NULL) goto error;

	registry->operationsSize = DEFAULT_OPERATIONS;
	registry->registeredOperations = 0;
	operations = malloc(sizeof(Operation *) * DEFAULT_OPERATIONS); 
	if (operations == NULL) goto error;
	registry->operations = operations;

	return registry;

	error:
		perror("Error initializing registry");
		free(registry);
		free(operations);

		return NULL;
}


bool registerOperation(Registry *registry, Operation *op) {
	if (registry->registeredOperations >= registry->operationsSize) {
		registry->operationsSize ++;
		Operation **tmp = realloc(registry->operations, registry->operationsSize);
		if (tmp == NULL) {
			perror("Error registering operation");
			return false;
		}

		registry->operations = tmp;
	}

	registry->operations[registry->registeredOperations] = op;
	registry->registeredOperations ++;

	return true;
}


Operation *_searchOperation(Registry const *registry, char symbol) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		if (registry->operations[i]->symbol == symbol) return registry->operations[i];
	}

	return NULL;
}


Operation const *searchOperation(Registry const *registry, char symbol) {
	return _searchOperation(registry, symbol);
}


bool addOperationImplementation(Registry *registry, char symbol, BuiltinImplementation fn){
	Operation *op = _searchOperation(registry, symbol);
	if (op == NULL) {
		perror("Error adding operation implementation");
		return false;
	}

	if (op->nImplementations >= op->implementationSize) { 
		op->implementationSize ++;
		BuiltinImplementation *temp = realloc(op->implementations, sizeof(BuiltinImplementation) * op->implementationSize);
		if (temp == NULL) {
			perror("Error registering operation implementation");
			return 0;
		}

		op->implementations = temp;
	}
	op->implementations[op->nImplementations] = fn;
	op->nImplementations ++;
	return 1;
}


bool registerObject(
		Registry *registry, char const *identifier, char const *originModule,
		void (*cleanup) (void *data), int32_t (*compare) (void const *ptr)
) {
	if (registry->registeredObjects >= registry->objectsSize) {
		registry->objectsSize ++;
		ObjectRegistry *tmp = realloc(registry->objects, registry->objectsSize);
		if (tmp == NULL) {
			perror("Error registering object");
			return false;
		}

		registry->objects = tmp;
	}

	ObjectRegistry reg = {
		.identifier	= strdup(identifier),
		.cleanup = cleanup,
		.compare = compare
	};

	registry->objects[registry->registeredObjects] = reg;
	registry->registeredObjects ++;

	return true;
}


static const Object *searchObjectID(const Registry *registry, uint32_t id) {
	return NULL;
}


const Object *searchObject(const Registry *registry, const char *id) {
	return NULL;
}


void freeRegistry(Registry *registry) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		free(registry->operations[i]->implementations);
		free(registry->operations[i]);
	}
	free(registry->operations);

	free(registry);
}
