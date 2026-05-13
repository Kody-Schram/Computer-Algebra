#include "objects.h"
#include "core/primitives/types.h"
#include <string.h>



bool createObject(
		char const identifier[REFERENCE_LENGTH], char const originModule[REFERENCE_LENGTH],
		void (*cleanup)(void *data), uint32_t (*compare)(void const *a, void const *b)
) {
	// Converts passed in string array into far faster 64 bit integers
	uint64_t id;
	memcpy(&id, identifier, REFERENCE_LENGTH);

	uint64_t origin;
	memcpy(&origin, originModule, REFERENCE_LENGTH);


}


bool initObjects();
