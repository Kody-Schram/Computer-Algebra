#pragma once

#include "core/primitives/types.h"
#include <stdint.h>


bool createObject(
		Object *out,
		char const *identifier, char const *originModule,
		void (*cleanup)(void *data), int32_t (*compare)(void const *a, void const *b)
); 


bool initObjects();

