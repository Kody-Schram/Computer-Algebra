#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "registry.h"
#include "core/common.h"
#include "core/context.h"
#include "core/primitives/numbers.h"
#include "core/utils/log.h"


#define DEFAULT_OPERATIONS 5 // +, -, *, /, ^
#define DEFAULT_OPERATION_IMPLEMENTATIONS 2
#define DEFAULT_OBJECTS 3


Registry *initRegistry() {
	Registry *registry = NULL;
	Operation *operations = NULL;
	char *op_mapping = NULL;
	Object *objects = NULL;
	uint64_t *obj_mapping = NULL;

	registry = malloc(sizeof(Registry));
	if (registry == NULL) goto error;

	registry->operationsSize = DEFAULT_OPERATIONS;
	registry->registeredOperations = 0;

	operations = malloc(sizeof(Operation) * DEFAULT_OPERATIONS); 
	op_mapping = malloc(sizeof(char) * DEFAULT_OPERATIONS);
	if (operations == NULL || op_mapping == NULL) goto error;

	registry->operations = operations;
	registry->operation_mapping = op_mapping;

	registry->objectsSize = DEFAULT_OBJECTS;
	registry->registeredObjects = 0;

	objects = malloc(sizeof(Object) * DEFAULT_OBJECTS);
	obj_mapping = malloc(sizeof(uint64_t) * DEFAULT_OBJECTS);

	if (objects == NULL || obj_mapping == NULL) goto error;

	registry->objects = objects;
	registry->object_mapping = obj_mapping;

	registry->numberParser = defaultNumberParser;

	return registry;

	error:
		perror("Error initializing registry");
		free(registry);
		free(operations);
		free(op_mapping);
		free(objects);
		free(obj_mapping);

		return NULL;
}


bool registerOperation(Registry *registry, Operation op) {
	if (registry->registeredOperations >= registry->operationsSize) {
		registry->operationsSize ++;
		Operation *op_tmp = realloc(registry->operations, sizeof(Operation) * registry->operationsSize);
		char *map_tmp = realloc(registry->operation_mapping, sizeof(char) * registry->operationsSize);
		if (op_tmp == NULL || map_tmp == NULL) {
			perror("Error registering operation");
			return false;
		}

		registry->operations = op_tmp;
		registry->operation_mapping = map_tmp;
	}

	registry->operations[registry->registeredOperations] = op;
	registry->operation_mapping[registry->registeredOperations] = op.symbol;
	registry->registeredOperations ++;

	return true;
}


static Operation *_searchOperation(Registry const *registry, char symbol) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		if (registry->operation_mapping[i] == symbol) return &registry->operations[i];
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


bool registerObject(Registry *registry, Object obj, uint64_t id) {
	if (registry->registeredObjects >= registry->objectsSize) {
		registry->objectsSize ++;

		Object *obj_tmp = realloc(registry->objects, sizeof(Object) * registry->objectsSize);
		uint64_t *id_tmp = realloc(registry->object_mapping, sizeof(uint64_t) * registry->objectsSize);
		
		if (obj_tmp == NULL || id_tmp == NULL) {
			perror("Error registering object");
			return false;
		}

		registry->objects = obj_tmp;
		registry->object_mapping = id_tmp;
	}

	registry->objects[registry->registeredObjects] = obj;
	registry->object_mapping[registry->registeredObjects] = id;
	registry->registeredObjects ++;

	return true;
}


Object const *searchObject(Registry const *registry, uint64_t obj_id) {
	for (uint32_t i = 0; i < registry->registeredObjects; i ++) {
		if (registry->object_mapping[i] == obj_id) return &(registry->objects[i]);
	}
	
	return NULL;
}


void freeRegistry(Registry *registry) {
	free(registry->objects);
	free(registry->object_mapping);

	for (uint32_t i = 0; i < registry->registeredOperations; i ++) {
		free(registry->operations[i].implementations);
	}
	free(registry->operations);
	free(registry->operation_mapping);

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



bool createObject(Object *out, uint64_t originModule,
		void (*cleanup)(ObjectValue value, uint32_t flags),
		int32_t (*compare)(ObjectValue const a, uint32_t aFlags, ObjectValue const b, uint32_t bFlags),
		bool (*copy)(ObjectValue const src, ObjectValue *dest, uint32_t flags),
		char *(*print)(ObjectValue const value, uint32_t flags)
) {

	(*out) = (Object) {
		.module = originModule,
		.cleanup = cleanup,
		.compare = compare,
		.copy = copy,
		.print = print
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

	initNumbers(registry);

	return true;
}
