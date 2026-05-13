#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


bool createObject(
		char const identifier[REFERENCE_LENGTH], char const originModule[REFERENCE_LENGTH],
		void (*cleanup)(void *data), uint32_t (*compare)(void const *a, void const *b)
); 


bool initObjects();

