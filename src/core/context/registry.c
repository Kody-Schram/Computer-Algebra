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


/*
 * CREATE_LOOKUP_128 defines a commutative and injective operation to create a faster lookup
 * (might also be surjective I just didnt check since its irrelevant here)
 *
 * CREATE_LOOKUP_128 is commutative since boolean |(OR) and &(AND) are commutative
 *
 * Let a,b,c,d be uint64_t such that CREATE_lOOKUP_128(a,b) = CREATE_LOOKUP_128(c, d)
 * So, (a | b, a & b) = (c | d, c & d)
 * => a | b = c | d and a & b = c & d
 * 
 * Then, a | b = c | d 
 * => a & (a | b) = a & (c | d)
 * => a = (a & c) | (a & d)
 *
 * Similarily, c = (c & a) | (c & b)
 *
 * Let a/b/c/d = 1 represent the bit being on within a/b/c/d
 * and similarily a/b/c/d = 0 representing the bit being off within a/b/c/d
 *
 * Case 1: a = 1, b = 1
 * Then, 1 = (1 & c) | (1 & d) => c | d = 1. Since a & b = c & d => c = 1, d = 1
 * So {a, b} = {c, d}
 *
 * Case 2: a = 1, b = 0
 * Then 1 = (1 & c) | (1 & d) => c = 1 or d = 1. Since a & b = c & d, c = 1 xor d = 1.
 * Since CREATE_LOOKUP_128 is commutative, (a, b) = (c, d) regardless of which c xor d is 1.
 * So {a, b} = {c, d}
 *
 * Case 3: a = 0, b = 0
 * Then, c = (c & 0) | (c & 0) => c = 0. Since a | b = c | d => d = 0.
 * So {a, b} = {c, d}
 *
 * And so CREATE_LOOKUP_128 is injective and will have no collisions.
 */
#define CREATE_LOOKUP_128(a,b) \
	(uint128_t) {.bits = {((uint64_t) (a | b)), ((uint64_t) (a & b))}}


Registry *initRegistry() {
	Registry *registry = NULL;
	Operation *operations = NULL;
	char *op_map = NULL;
	Object *objects = NULL;
	uint64_t *obj_map = NULL;

	registry = malloc(sizeof(Registry));
	if (registry == NULL) goto error;

	registry->operationsSize = DEFAULT_OPERATIONS;
	registry->registeredOperations = 0;

	operations = malloc(sizeof(Operation) * DEFAULT_OPERATIONS); 
	op_map = malloc(sizeof(char) * DEFAULT_OPERATIONS);
	if (operations == NULL || op_map == NULL) goto error;

	registry->operations = operations;
	registry->operation_map = op_map;

	registry->objectsSize = DEFAULT_OBJECTS;
	registry->registeredObjects = 0;

	objects = malloc(sizeof(Object) * DEFAULT_OBJECTS);
	obj_map = malloc(sizeof(uint64_t) * DEFAULT_OBJECTS);

	if (objects == NULL || obj_map == NULL) goto error;

	registry->objects = objects;
	registry->object_map = obj_map;

	registry->numberParser = defaultNumberParser;

	return registry;

	error:
		perror("Error initializing registry");
		free(registry);
		free(operations);
		free(op_map);
		free(objects);
		free(obj_map);

		return NULL;
}


bool registerOperation(Registry *registry, Operation op) {
	if (registry->registeredOperations >= registry->operationsSize) {
		registry->operationsSize ++;
		Operation *op_tmp = realloc(registry->operations, sizeof(Operation) * registry->operationsSize);
		char *map_tmp = realloc(registry->operation_map, sizeof(char) * registry->operationsSize);
		if (op_tmp == NULL || map_tmp == NULL) {
			perror("Error registering operation");
			return false;
		}

		registry->operations = op_tmp;
		registry->operation_map= map_tmp;
	}

	registry->operations[registry->registeredOperations] = op;
	registry->operation_map[registry->registeredOperations] = op.symbol;
	registry->registeredOperations ++;

	return true;
}


static Operation *_searchOperation(Registry const *registry, char symbol) {
	for (unsigned int i = 0; i < registry->registeredOperations; i ++) {
		if (registry->operation_map[i] == symbol) return &registry->operations[i];
	}

	return NULL;
}


Operation const *searchOperation(Registry const *registry, char symbol) {
	return _searchOperation(registry, symbol);
}


bool addOperationImplementation(
		Registry *registry, char symbol, 
		OperationImplementation implementation, uint64_t ids[2]
) {
	Operation *op = _searchOperation(registry, symbol);
	if (op == NULL) {
		perror("Error adding operation implementation");
		return false;
	}

	if (op->nImplementations >= op->implementationSize) { 
		op->implementationSize ++;
		OperationImplementation *temp = realloc(op->implementations, sizeof(OperationImplementation) * op->implementationSize);
		uint128_t *temp_map = realloc(op->implementation_map, sizeof(uint128_t) * op->implementationSize);
		if (temp == NULL || temp_map == NULL) {
			perror("Error registering operation implementation");
			return false;
		}

		op->implementations = temp;
		op->implementation_map = temp_map;
	}
	op->implementations[op->nImplementations] = implementation;
	op->implementation_map[op->nImplementations] = CREATE_LOOKUP_128(ids[0], ids[1]);
	op->nImplementations ++;
	return true;
}


BuiltinResult dispatchOperation(Operation const *op, 
		uint64_t id_a, ObjectData a, uint64_t id_b, ObjectData b, 
		ObjectData *out, uint64_t *id_out
) {
	uint128_t lookup = CREATE_LOOKUP_128(id_a, id_b);
	for (uint32_t i = 0; i < op->nImplementations; i ++) {
		if (COMPARE_UINT128_T(lookup, op->implementation_map[i]))
			return op->implementations[i](GLOBALCONTEXT, a, b, out, id_out);
	}

	return BUILTIN_NEUTRAL;
}

bool registerObject(
		Registry *registry, uint64_t originModule, uint64_t id,
		void (*cleanup)(ObjectData data),
		int32_t (*compare)(ObjectData const a, ObjectData const b),
		bool (*copy)(ObjectValue const src, ObjectValue *dest, ExpressionMeta meta, uint32_t flags),
		char *(*print)(ObjectData const data)
) {
	if (registry->registeredObjects >= registry->objectsSize) {
		uint32_t newSize = registry->objectsSize + 1;

        Object *obj_tmp = realloc(registry->objects, sizeof(Object) * newSize);
        if (obj_tmp == NULL) {
            perror("Error reallocating objects array");
            return false;
        }
        registry->objects = obj_tmp; 

        uint64_t *id_tmp = realloc(registry->object_map, sizeof(uint64_t) * newSize);
        if (id_tmp == NULL) {
            perror("Error reallocating object map array");
            return false;
        }
        registry->object_map = id_tmp;
        registry->objectsSize = newSize;
	}

	registry->objects[registry->registeredObjects] = (Object) {
		.module = originModule,
		.cleanup = cleanup,
		.compare = compare,
		.copy = copy,
		.print = print
	};
	registry->object_map[registry->registeredObjects] = id;
	registry->registeredObjects ++;

	return true;
}


Object const *searchObject(Registry const *registry, uint64_t obj_id) {
	for (uint32_t i = 0; i < registry->registeredObjects; i ++) {
		if (registry->object_map[i] == obj_id) return &(registry->objects[i]);
	}
	
	return NULL;
}


void freeRegistry(Registry *registry) {
	free(registry->objects);
	free(registry->object_map);

	for (uint32_t i = 0; i < registry->registeredOperations; i ++) {
		free(registry->operations[i].implementations);
		free(registry->operations[i].implementation_map);
	}
	free(registry->operations);
	free(registry->operation_map);

	free(registry);
}


bool createOperation(Operation *out, const char symbol, Associativity a, bool c, uint32_t precedence) {
	OperationImplementation *implementations = malloc(sizeof(OperationImplementation) * DEFAULT_OPERATION_IMPLEMENTATIONS);
	if (implementations == NULL) {
		perror("Error creating operation");
		return false;
	}

	uint128_t *implementation_map = malloc(sizeof(uint128_t) * DEFAULT_OPERATION_IMPLEMENTATIONS);
	if (implementation_map == NULL) {
		perror("Error creating operation");
		return false;
	}

	(*out) = (Operation) {
		.symbol = symbol,
		.associativity = a,
		.commutative = c,
		.precedence = precedence,
		.implementationSize = DEFAULT_OPERATION_IMPLEMENTATIONS,
		.nImplementations = 0,
		.implementation_map = implementation_map,
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

	initNumbers(registry);

	return true;
}
