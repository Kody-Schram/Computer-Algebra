#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "objects.h"
#include "core/primitives/types.h"


bool createObject(
		Object *out,
		char const *identifier, char const *originModule,
		void (*cleanup)(void *data), int32_t (*compare)(void const *a, void const *b)
) {
	// Converts passed in string array into far faster 64 bit integers
	uint64_t id;
	for (uint16_t i = 0; i < REFERENCE_LENGTH; i ++) {
		if (identifier[i] == '\0') {
			printf("Identifier of length %d, should be standard length %d", i, REFERENCE_LENGTH);
			return false;
		}
	}

	memcpy(&id, identifier, REFERENCE_LENGTH);

	uint64_t origin;

	for (uint16_t i = 0; i < REFERENCE_LENGTH; i ++) {
		if (originModule[i] == '\0') {
			printf("Origin module of length %d, should be standard length %d", i, REFERENCE_LENGTH);
			return false;
		}
	}

	memcpy(&origin, originModule, REFERENCE_LENGTH);

	*out = (Object) {
		.id = id,
		.module = origin,
		.cleanup = cleanup,
		.compare = compare
	};

	return true;	
}


bool initObjects();
