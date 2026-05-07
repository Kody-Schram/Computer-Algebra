#pragma once

#include <stdint.h>


bool createObject(
		char const *identifier, char const *originModule,
		void (*cleanup)(void *data), uint32_t (*compare)(void const *a, void const *b)
); 


bool initObjects();

