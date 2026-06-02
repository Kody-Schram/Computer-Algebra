#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core/context.h"
#include "registry.h"
#include "core/primitives/integers.h"
#include "core/utils/log.h"
#include "core/primitives/types.h"


#define DEFAULT_OPERATIONS 5 // +, -, *, /, ^
#define DEFAULT_OPERATION_IMPLEMENTATIONS 2
#define DEFAULT_OBJECTS 3


Registry *initRegistry() {
	Registry *registry = NULL;
	Operation *operations = NULL;
	Object *objects = NULL;

	registry = malloc(sizeof(Registry));
	if (registry == NULL) goto error;

	registry->operationsSize = DEFAULT_OPERATIONS;
	registry->registeredOperations = 0;
	operations = malloc(sizeof(Operation) * DEFAULT_OPERATIONS); 
	if (operations == NULL) goto error;
	registry->operations = operations;

	registry->objectsSize = DEFAULT_OBJECTS;
	registry->registeredObjects = 0;
	objects = malloc(sizeof(Object) * DEFAULT_OBJECTS);
	if (objects == NULL) goto error;
	registry->objects = objects;

	return registry;

	error:
		perror("Error initializing registry");
		free(registry);
		free(operations);
		free(objects);

		return NULL;
}


bool registerOperation(Registry *registry, Operation op) {
	if (registry->registeredOperations >= registry->operationsSize) {
		registry->operationsSize ++;
		Operation *tmp = realloc(registry->operations, registry->operationsSize);
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


static Operation *_searchOperation(Registry const *registry, char symbol) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		if (registry->operations[i].symbol == symbol) return &registry->operations[i];
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
			return false;
		}

		op->implementations = temp;
	}
	op->implementations[op->nImplementations] = fn;
	op->nImplementations ++;
	return true;
}


bool registerObject(Registry *registry, Object obj) {
	if (registry->registeredObjects >= registry->objectsSize) {
		registry->objectsSize ++;
		Object *tmp = realloc(registry->objects, registry->objectsSize);
		if (tmp == NULL) {
			perror("Error registering object");
			return false;
		}

		registry->objects = tmp;
	}

	registry->objects[registry->registeredObjects] = obj;
	registry->registeredObjects ++;

	return true;
}


static Object const *searchObjectID(Registry const *registry, uint64_t obj_id) {
	for (uint32_t i = 0; i < registry->registeredObjects; i ++) {
		if (registry->objects[i].id == obj_id) return &registry->objects[i];
	}
	
	return NULL;
}


void freeRegistry(Registry *registry) {
	free(registry->objects);

	for (uint32_t i = 0; i < registry->registeredOperations; i ++) {
		free(registry->operations[i].implementations);
	}
	free(registry->operations);

	free(registry);
}


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t precedence) {
	BuiltinImplementation *implementations = malloc(sizeof(BuiltinImplementation) * DEFAULT_OPERATION_IMPLEMENTATIONS);
	if (implementations == NULL) {
		perror("Error creating operation");
		return false;
	}

	(*out) = (Operation) {
		.symbol = symbol,
		.associativity = a,
		.commutative = c,
		.arity = 2,
		.precedence = precedence,
		.implementationSize = DEFAULT_OPERATION_IMPLEMENTATIONS,
		.nImplementations = 0,
		.implementations = implementations
	};
	
	return true;
}

bool initPrimitives(Registry *registry) {
	Info(0, "Initalizing Primitives\n");
	Debug(0, "Initializing primitive operations\n");

	Operation add;
	if (!createOperation(&add, '+', ASSOC_BOTH, true, 1)) return false;
	if (!registerOperation(registry, add)) return false;

	Operation mult;
	if (!createOperation(&mult, '*', ASSOC_BOTH, true, 2)) return false;
	if (!registerOperation(registry, mult)) return false;

	Operation expo;
	if (!createOperation(&expo, '^', ASSOC_RIGHT, false, 3)) return false;
	if (!registerOperation(registry, expo)) return false;

   	Operation div;
	if (!createOperation(&div, '/', ASSOC_LEFT, false, 2)) return false;
	if (!registerOperation(registry, div)) return false;

	Operation sub;
	if (!createOperation(&sub, '-', ASSOC_LEFT, false, 1)) return false;
	if (!registerOperation(registry, sub)) return false;

	initIntegers(registry);

	return true;
}
