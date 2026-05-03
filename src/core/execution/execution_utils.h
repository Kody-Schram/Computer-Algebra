#pragma once

#include <stdint.h>

#include "core/primitives/types.h"


BuiltinResult callImplementations(
		uint32_t nImplementations, BuiltinImplementation *implementations,
		uint32_t nArgs, Expression **exprs		
);
