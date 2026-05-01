#include <stdlib.h>
#include <stdio.h>

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


int registerOperation(Registry *registry, Operation *op) {
	if (registry->registeredOperations >= registry->operationsSize) {
		registry->operationsSize += DEFAULT_OPERATIONS;
		Operation **tmp = realloc(registry->operations, registry->operationsSize);
		if (tmp == NULL) {
			perror("Error registering operation");
			return 0;
		}

		registry->operations = tmp;
	}

	registry->operations[registry->registeredOperations] = op;
	registry->registeredOperations ++;

	return 1;
}


Operation *searchOperation(Registry *registry, const char symbol) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		if (registry->operations[i]->symbol == symbol) return registry->operations[i];
	}

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
