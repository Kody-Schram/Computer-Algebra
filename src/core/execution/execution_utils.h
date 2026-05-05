#pragma once

#include <stdint.h>

#include "core/primitives/types.h"


BuiltinResult callImplementations(
		uint32_t nImplementations, BuiltinImplementation const * const implementations,
		Context const *ctx, uint32_t nArgs, Expression **exprs		
);
